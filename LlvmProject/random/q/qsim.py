#!/bin/env python

import uuid
import asyncio
import threading
import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import asynccontextmanager
from datetime import datetime, timezone
from typing import Optional

from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field
from qiskit.qasm3 import loads
from qiskit import transpile
from qiskit_aer import AerSimulator

# ---------------------------------------------------------------------------
# State
# ---------------------------------------------------------------------------

_sim = AerSimulator()
_executor = ThreadPoolExecutor()
_jobs: dict[str, dict] = {}
_lock = threading.Lock()


@asynccontextmanager
async def lifespan(app: FastAPI):
    yield
    _executor.shutdown(wait=False)


app = FastAPI(title="QSim", description="AerSimulator as a service", lifespan=lifespan)


# ---------------------------------------------------------------------------
# Schemas
# ---------------------------------------------------------------------------

class JobSubmit(BaseModel):
    qasm: str
    shots: int = Field(default=1024, gt=0)
    parameters: dict[str, float] = Field(default_factory=dict)


class JobResponse(BaseModel):
    job_id: str
    status: str
    created_at: str
    running_at: Optional[str]
    finished_at: Optional[str]
    shots: int
    error: Optional[str]


class JobResults(BaseModel):
    counts: dict[str, int]
    shots: int


# ---------------------------------------------------------------------------
# Simulation (runs in ThreadPoolExecutor — Aer releases GIL for C++ core)
# ---------------------------------------------------------------------------

def _now() -> str:
    return datetime.now(timezone.utc).isoformat()


def _simulate(job_id: str, qasm_str: str, shots: int, parameters: dict[str, float]):
    with _lock:
        if _jobs[job_id]["status"] == "CANCELLED":
            return
        _jobs[job_id]["status"] = "RUNNING"
        _jobs[job_id]["running_at"] = _now()

    try:
        circuit = loads(qasm_str)

        if circuit.parameters and parameters:
            param_map = {p: parameters[p.name] for p in circuit.parameters if p.name in parameters}
            circuit = circuit.assign_parameters(param_map)

        transpiled = transpile(circuit, _sim)
        result = _sim.run(transpiled, shots=shots).result()
        counts = result.get_counts(transpiled)

        with _lock:
            if _jobs[job_id]["status"] == "CANCELLED":
                return
            _jobs[job_id].update(
                status="DONE",
                finished_at=_now(),
                results={"counts": counts, "shots": shots},
            )
    except Exception as exc:
        print(f"[qsim] job {job_id} ERROR: {exc}", flush=True)
        with _lock:
            if _jobs[job_id]["status"] != "CANCELLED":
                _jobs[job_id].update(status="ERROR", finished_at=_now(), error=str(exc))


# ---------------------------------------------------------------------------
# Routes
# ---------------------------------------------------------------------------

@app.post("/jobs", status_code=201, response_model=JobResponse)
async def submit_job(body: JobSubmit):
    job_id = str(uuid.uuid4())
    created_at = _now()

    with _lock:
        _jobs[job_id] = {
            "job_id": job_id,
            "status": "QUEUED",
            "created_at": created_at,
            "running_at": None,
            "finished_at": None,
            "shots": body.shots,
            "error": None,
            "results": None,
        }

    loop = asyncio.get_event_loop()
    loop.run_in_executor(_executor, _simulate, job_id, body.qasm, body.shots, body.parameters)

    return _job_view(job_id)


@app.get("/jobs", response_model=list[JobResponse])
async def list_jobs():
    with _lock:
        ids = list(_jobs)
    return [_job_view(jid) for jid in ids]


@app.get("/jobs/{job_id}", response_model=JobResponse)
async def get_job(job_id: str):
    _assert_exists(job_id)
    return _job_view(job_id)


@app.get("/jobs/{job_id}/results", response_model=JobResults)
async def get_results(job_id: str):
    _assert_exists(job_id)
    with _lock:
        job = _jobs[job_id]

    match job["status"]:
        case "ERROR":
            raise HTTPException(status_code=500, detail=job["error"])
        case "CANCELLED":
            raise HTTPException(status_code=410, detail="job was cancelled")
        case "DONE":
            return JSONResponse(content=job["results"])
        case _:
            return JSONResponse(
                status_code=202,
                content={"status": job["status"], "message": "results not yet available"},
            )


@app.delete("/jobs/{job_id}", response_model=JobResponse)
async def cancel_job(job_id: str):
    _assert_exists(job_id)
    with _lock:
        job = _jobs[job_id]
        if job["status"] in {"DONE", "ERROR", "CANCELLED"}:
            raise HTTPException(status_code=409, detail=f"job already terminal: {job['status']}")
        _jobs[job_id]["status"] = "CANCELLED"
        _jobs[job_id]["finished_at"] = _now()

    return _job_view(job_id)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _assert_exists(job_id: str):
    with _lock:
        if job_id not in _jobs:
            raise HTTPException(status_code=404, detail="job not found")


def _job_view(job_id: str) -> dict:
    with _lock:
        job = _jobs[job_id]
    return {k: v for k, v in job.items() if k != "results"}


# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import uvicorn

    parser = argparse.ArgumentParser(description="AerSimulator HTTP service")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    uvicorn.run(app, host=args.host, port=args.port)
