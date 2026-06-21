#!/bin/env python
# Submit a QASM file to qsim, auto-detecting circuit type and optimizing with SPSA.
#
# Plain circuit (no input params):
#   qrun.py <circuit.qasm>
#
# QAOA (_theta_0_, _theta_1_) - SPSA over Ising Hamiltonian from same file:
#   qrun.py <foo_qaoa.qasm> [--iters N] [--opt-shots N]
#
# VQE (_θ_N_) - SPSA over Ising Hamiltonian from companion *_qaoa.qasm:
#   qrun.py <foo_vqe.qasm> [--iters N] [--opt-shots N]
#
# Works with all circuits in ~/iitk/C2Q/examples/

import sys
import re
import os
import math
import random
import time
import argparse
import requests


# ---------------------------------------------------------------------------
# Circuit type detection and parameter parsing
# ---------------------------------------------------------------------------

def detect_type(qasm: str) -> str:
    if re.search(r'input\s+float\[64\]\s+_theta_\d+_', qasm):
        return 'qaoa'
    if re.search(r'input\s+float\[64\]\s+_θ_\d+_', qasm):
        return 'vqe'
    return 'plain'


def parse_qaoa_params(qasm: str) -> list[str]:
    return re.findall(r'input\s+float\[64\]\s+(_theta_\d+_)\s*;', qasm)


def parse_vqe_params(qasm: str) -> list[str]:
    found = re.findall(r'input\s+float\[64\]\s+(_θ_(\d+)_)\s*;', qasm)
    return [name for name, _ in sorted(found, key=lambda x: int(x[1]))]


# ---------------------------------------------------------------------------
# Ising Hamiltonian extraction from QAOA QASM
#
# Handles two circuit forms produced by different compilers:
#
# Form A - explicit gate calls (C2Q / qiskit-terra style):
#   rzz(c*_theta_0_) q[i], q[j]  → J[(i,j)] += c/2
#   rz(c*_theta_0_)  q[i]        → h[i]     += c/2
#
# Form B - decomposed (cx / U(0,0,…) / cx):
#   cx q[A], q[B]; U(0,0,c*_theta_0_) q[B]; cx q[A], q[B]  → J[(A,B)] += c/2
#   Supports interleaved decompositions (shared cx layers in optimised circuits).
#   Standalone U(0,0,c*_theta_0_) q[i] not between a matching cx pair → h[i] += c/2
#
# (factor of /2 from the rzz/rz gate convention: rzz(θ) ≡ e^{-iθ/2 ZZ})
# ---------------------------------------------------------------------------

def _parse_coeff(expr: str) -> float:
    expr = expr.strip()
    m = re.match(r'^(-?[\d.]+)\*_theta_0_$', expr)
    if m:
        return float(m.group(1))
    if expr == '_theta_0_':
        return 1.0
    if expr == '-_theta_0_':
        return -1.0
    return 1.0


def parse_hamiltonian(qaoa_qasm: str) -> tuple[dict, dict]:
    h, J = {}, {}

    # --- Form A: explicit rzz / rz calls ---
    for m in re.finditer(r'rzz\((-?[\d.]+)\*_theta_0_\)\s+q\[(\d+)\],\s*q\[(\d+)\]', qaoa_qasm):
        i, j = int(m.group(2)), int(m.group(3))
        k = (min(i, j), max(i, j))
        J[k] = J.get(k, 0.0) + float(m.group(1)) / 2
    for m in re.finditer(r'\brz\((-?[\d.]+)\*_theta_0_\)\s+q\[(\d+)\]', qaoa_qasm):
        i = int(m.group(2))
        h[i] = h.get(i, 0.0) + float(m.group(1)) / 2

    if h or J:
        return h, J

    # --- Form B: decomposed cx / U(0,0,expr) / cx ---
    # State machine: pending[B] = (A, coeff_or_None) while rzz(A,B) is open.
    # A closing cx q[A],q[B] is identified by matching A and B with a pending
    # entry that already has its U gate recorded.
    pending: dict[int, tuple[int, float | None]] = {}

    cx_re  = re.compile(r'\s*cx\s+q\[(\d+)\],\s*q\[(\d+)\]\s*;')
    u_re   = re.compile(r'\s*U\(0\s*,\s*0\s*,\s*([^)]+)\)\s+q\[(\d+)\]\s*;')
    rz_re  = re.compile(r'\s*rz\(([^)]+)\)\s+q\[(\d+)\]\s*;')

    for line in qaoa_qasm.splitlines():
        if m := cx_re.match(line):
            A, B = int(m.group(1)), int(m.group(2))
            if B in pending and pending[B][0] == A and pending[B][1] is not None:
                # closing cx: commit J term
                coeff = pending.pop(B)[1]
                k = (min(A, B), max(A, B))
                J[k] = J.get(k, 0.0) + coeff / 2
            else:
                pending[B] = (A, None)      # opening cx
            continue

        gate_m = u_re.match(line) or rz_re.match(line)
        if gate_m:
            expr, B = gate_m.group(1), int(gate_m.group(2))
            if '_theta_0_' not in expr:
                continue
            coeff = _parse_coeff(expr)
            if B in pending and pending[B][1] is None:
                pending[B] = (pending[B][0], coeff)   # attach to open rzz
            else:
                h[B] = h.get(B, 0.0) + coeff / 2     # standalone diagonal term

    return h, J


def ising_energy(counts: dict, h: dict, J: dict) -> float:
    total = sum(counts.values())
    energy = 0.0
    for bitstr, cnt in counts.items():
        bits = bitstr.replace(' ', '')
        Z = [1 - 2 * int(b) for b in reversed(bits)]   # Z[i] = ±1 for q[i]
        e = sum(h.get(i, 0.0) * Z[i] for i in h)
        e += sum(v * Z[i] * Z[j] for (i, j), v in J.items())
        energy += cnt * e
    return energy / total


# ---------------------------------------------------------------------------
# Parameter binding - text substitution, same approach as example scripts
# ---------------------------------------------------------------------------

def bind_params(qasm: str, param_names: list[str], theta: list[float]) -> str:
    s = qasm
    for name, val in zip(param_names, theta):
        s = re.sub(rf'input\s+float\[64\]\s+{re.escape(name)}\s*;\n', '', s)
        s = s.replace(name, repr(val))
    return s


# ---------------------------------------------------------------------------
# Companion QAOA file for VQE (extracts Hamiltonian)
# foo_vqe.qasm  →  foo_qaoa.qasm (same directory)
# ---------------------------------------------------------------------------

def find_companion_qaoa(vqe_path: str) -> str | None:
    companion = re.sub(r'_vqe\.qasm$', '_qaoa.qasm', vqe_path)
    if companion != vqe_path and os.path.exists(companion):
        return companion
    # fallback: any *_qaoa.qasm in same directory
    d = os.path.dirname(vqe_path) or '.'
    candidates = [os.path.join(d, f) for f in os.listdir(d) if f.endswith('_qaoa.qasm')]
    return candidates[0] if len(candidates) == 1 else None


# ---------------------------------------------------------------------------
# HTTP helpers
# ---------------------------------------------------------------------------

def submit(base: str, qasm: str, shots: int) -> str:
    resp = requests.post(f"{base}/jobs", json={"qasm": qasm, "shots": shots, "parameters": {}})
    resp.raise_for_status()
    return resp.json()["job_id"]


def poll(base: str, job_id: str, verbose: bool = True, interval: float = 0.5) -> str:
    while True:
        resp = requests.get(f"{base}/jobs/{job_id}")
        resp.raise_for_status()
        status = resp.json()["status"]
        if verbose:
            print(f"[qrun] {status}", flush=True)
        if status in {"DONE", "ERROR", "CANCELLED"}:
            return status
        time.sleep(interval)


def fetch_results(base: str, job_id: str) -> dict:
    resp = requests.get(f"{base}/jobs/{job_id}/results")
    resp.raise_for_status()
    return resp.json()


def evaluate(base: str, bound_qasm: str, shots: int, h: dict, J: dict) -> float:
    job_id = submit(base, bound_qasm, shots)
    status = poll(base, job_id, verbose=False)
    if status != "DONE":
        raise RuntimeError(f"job {job_id} ended with status={status}")
    counts = fetch_results(base, job_id)["counts"]
    return ising_energy(counts, h, J)


# ---------------------------------------------------------------------------
# SPSA - minimizes Ising energy (gradient descent, gradient-free)
#
# FUTURE: replace with COBYLA (scipy) for VQE if scipy becomes available;
# COBYLA handles many params more efficiently with its trust-region approach.
# Signature would be:
#   from scipy.optimize import minimize
#   res = minimize(cost_fn, x0, method='COBYLA', options={'maxiter': 300, 'rhobeg': 0.5})
# ---------------------------------------------------------------------------

def spsa_optimize(
    base: str, qasm: str, param_names: list[str], h: dict, J: dict,
    shots: int, max_iter: int,
    a: float, c: float, A: float, alpha: float, gamma: float,
    init: list[float],
    patience: int, min_delta: float,
) -> tuple[list[float], float]:
    n = len(param_names)
    theta = init[:]

    print(f"[spsa] {n} params, {max_iter} iters × 2 evals @ {shots} shots  (patience={patience})", flush=True)
    print(f"[spsa] init  theta = {[f'{t:.4f}' for t in theta]}\n", flush=True)

    best_energy  = math.inf
    best_theta   = theta[:]
    no_improve   = 0

    for k in range(max_iter):
        a_k = a / (A + k + 1) ** alpha
        c_k = c / (k + 1) ** gamma
        delta = [random.choice([-1.0, 1.0]) for _ in range(n)]

        theta_plus  = [theta[i] + c_k * delta[i] for i in range(n)]
        theta_minus = [theta[i] - c_k * delta[i] for i in range(n)]

        f_plus  = evaluate(base, bind_params(qasm, param_names, theta_plus),  shots, h, J)
        f_minus = evaluate(base, bind_params(qasm, param_names, theta_minus), shots, h, J)

        # gradient descent on Ising energy
        grad  = [(f_plus - f_minus) / (2 * c_k * delta[i]) for i in range(n)]
        theta = [theta[i] - a_k * grad[i] for i in range(n)]

        avg_e = (f_plus + f_minus) / 2
        if avg_e < best_energy - min_delta:
            best_energy = avg_e
            best_theta  = theta[:]
            no_improve  = 0
        else:
            no_improve += 1

        print(
            f"[spsa] {k+1:3d}/{max_iter}  E={avg_e:+.3f}  best={best_energy:+.3f}"
            f"  theta={[f'{t:.3f}' for t in theta]}",
            flush=True,
        )

        if no_improve >= patience:
            print(f"[spsa] early stop: no improvement for {patience} consecutive iters", flush=True)
            break

    return best_theta, best_energy


# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------

def print_results(counts: dict, h: dict | None = None, J: dict | None = None):
    total = sum(counts.values())
    ranked = sorted(counts.items(), key=lambda kv: kv[1], reverse=True)
    width = max(len(b.replace(' ', '')) for b in counts)

    has_ham = h is not None and J is not None and (h or J)
    header = f"{'bitstring':<{width}}   {'shots':>6}   {'prob':>6}"
    sep_w  = width + 18
    if has_ham:
        header += f"   {'E[H]':>7}"
        sep_w  += 11
    print(f"\n{header}")
    print("-" * sep_w)

    for bitstr, count in ranked:
        bits = bitstr.replace(' ', '')
        line = f"{bits:<{width}}   {count:>6}   {count/total:>6.2%}"
        if has_ham:
            Z = [1 - 2 * int(b) for b in reversed(bits)]
            e = sum(h.get(i, 0.0) * Z[i] for i in h)
            e += sum(v * Z[i] * Z[j] for (i, j), v in J.items())
            line += f"   {e:+7.3f}"
        print(line)

    best = ranked[0][0].replace(' ', '')
    print(f"\nmost frequent : |{best}⟩  (decimal: {int(best, 2)})")
    if has_ham:
        Z = [1 - 2 * int(b) for b in reversed(best)]
        e_best = sum(h.get(i, 0.0) * Z[i] for i in h)
        e_best += sum(v * Z[i] * Z[j] for (i, j), v in J.items())
        e_exp  = ising_energy(counts, h, J)
        print(f"H (best state): {e_best:+.4f}")
        print(f"E[H] (expect) : {e_exp:+.4f}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Submit a QASM file to qsim - auto-detects QAOA/VQE and optimizes"
    )
    parser.add_argument("file", help="path to .qasm file")
    parser.add_argument("--host",      default="127.0.0.1")
    parser.add_argument("--port",      type=int, default=8080)
    parser.add_argument("--shots",     type=int, default=2048, help="shots for the final run")
    parser.add_argument("--opt-shots", type=int, default=256,  help="shots per SPSA eval (default: 256)")
    parser.add_argument("--iters",     type=int, default=0,
                        help="SPSA iterations (default: auto: max(40, 20×n_params))")
    parser.add_argument("--spsa-a",    type=float, default=0.1)
    parser.add_argument("--spsa-c",    type=float, default=0.3)
    parser.add_argument("--spsa-A",    type=float, default=5.0)
    parser.add_argument("--spsa-alpha",type=float, default=0.602)
    parser.add_argument("--spsa-gamma",type=float, default=0.101)
    parser.add_argument("--patience",  type=int,   default=8,
                        help="stop after this many iters with no improvement (default: 8)")
    parser.add_argument("--min-delta", type=float, default=1e-3,
                        help="minimum energy drop to count as improvement (default: 1e-3)")
    args = parser.parse_args()

    base = f"http://{args.host}:{args.port}"

    with open(args.file) as f:
        qasm = f.read()

    circuit_type = detect_type(qasm)
    print(f"[qrun] circuit type: {circuit_type}  file: {args.file}", flush=True)

    h, J = {}, {}

    if circuit_type == 'plain':
        job_id = submit(base, qasm, args.shots)
        print(f"[qrun] submitted  job_id={job_id}", flush=True)
        status = poll(base, job_id)
        if status != "DONE":
            print(f"[qrun] job ended with status={status}", file=sys.stderr)
            sys.exit(1)
        print_results(fetch_results(base, job_id)["counts"])
        return

    # --- variational circuit: extract Hamiltonian, run SPSA ---

    if circuit_type == 'qaoa':
        param_names = parse_qaoa_params(qasm)
        h, J = parse_hamiltonian(qasm)
        init = [math.pi / 4] * len(param_names)

    else:  # vqe
        param_names = parse_vqe_params(qasm)
        companion   = find_companion_qaoa(args.file)
        if not companion:
            print("[qrun] cannot find companion *_qaoa.qasm for Hamiltonian - needed for VQE optimization",
                  file=sys.stderr)
            sys.exit(1)
        print(f"[qrun] Hamiltonian from: {companion}", flush=True)
        with open(companion) as f:
            h, J = parse_hamiltonian(f.read())
        # random init over [0, 2π] like the example vqe_sim scripts
        random.seed(0)
        init = [random.uniform(0, 2 * math.pi) for _ in param_names]

    if not (h or J):
        print("[qrun] WARNING: no rz/rzz terms found in QAOA QASM - Hamiltonian is empty", file=sys.stderr)

    n = len(param_names)
    max_iter = args.iters if args.iters > 0 else max(40, 20 * n)
    print(f"[qrun] {n} params, SPSA iters={max_iter}", flush=True)

    best_theta, best_energy = spsa_optimize(
        base, qasm, param_names, h, J,
        shots    = args.opt_shots,
        max_iter = max_iter,
        a        = args.spsa_a,
        c        = args.spsa_c,
        A        = args.spsa_A,
        alpha    = args.spsa_alpha,
        gamma    = args.spsa_gamma,
        init     = init,
        patience  = args.patience,
        min_delta = args.min_delta,
    )

    print(f"\n[spsa] converged  E={best_energy:+.4f}  theta={[f'{t:.4f}' for t in best_theta]}", flush=True)
    print(f"[qrun] final run: {args.shots} shots\n", flush=True)

    bound_qasm = bind_params(qasm, param_names, best_theta)
    job_id = submit(base, bound_qasm, args.shots)
    print(f"[qrun] submitted  job_id={job_id}", flush=True)
    status = poll(base, job_id)

    if status != "DONE":
        print(f"[qrun] job ended with status={status}", file=sys.stderr)
        sys.exit(1)

    counts = fetch_results(base, job_id)["counts"]
    print_results(counts, h, J)


if __name__ == "__main__":
    main()
