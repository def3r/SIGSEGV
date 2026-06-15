import sys
import time
import math
import json
from collections import Counter
from qiskit.qasm3 import loads
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2

# decoders

def add(data, edges):
    register = list(data.__dict__.keys())[0]
    counts = getattr(data, register).get_counts()
    most_frequent = max(counts, key=counts.get)
    return str(int(most_frequent, 2))

def mul(data, edges):
    register = list(data.__dict__.keys())[0]
    counts = getattr(data, register).get_counts()
    most_frequent = max(counts, key=counts.get)
    return str(int(most_frequent, 2))

def maxcut(data, edges):
    register = list(data.__dict__.keys())[0]
    counts = getattr(data, register).get_counts()
    top = Counter(counts).most_common(4)
    print("\nTop measurement outcomes (bitstring → count):", file=sys.stderr)
    best_cut = -1
    for bitstr, count in top:
        bits = bitstr.replace(" ", "")
        cut = sum(1 for u, v in edges if bits[-(u+1)] != bits[-(v+1)])
        partition_0 = [i for i, b in enumerate(reversed(bits)) if b == "0"]
        partition_1 = [i for i, b in enumerate(reversed(bits)) if b == "1"]
        print(f"  |{bits}> : {count:4d}   S0={partition_0}  S1={partition_1}  cut={cut}", file=sys.stderr)
        if cut > best_cut:
            best_cut = cut
    return str(best_cut)

# IPC helpers

def read_until_null():
    buf = b""
    while True:
        ch = sys.stdin.buffer.read(1)
        if ch == b'\x00' or ch == b'':
            return buf.decode()
        buf += ch

def send(msg):
    sys.stdout.buffer.write(msg.encode() + b'\x00')
    sys.stdout.buffer.flush()

# init
def init():
    service = QiskitRuntimeService()
    backend = service.least_busy(operational=True, simulator=False)
    pm = generate_preset_pass_manager(backend=backend, optimization_level=3)
    sampler = SamplerV2(backend)
    return pm, sampler

# main loop
def run(pm, sampler):
    while True:
        decoder_name = read_until_null()
        qasm_str     = read_until_null()

        if not decoder_name or not qasm_str:
            break

        try:
            circuit = loads(qasm_str)
            if circuit.parameters:
                param_map = {}
                for p in circuit.parameters:
                    if "_theta_0_" in p.name:
                        param_map[p] = math.pi / 4
                    elif "_theta_1_" in p.name:
                        param_map[p] = math.pi / 4
                circuit = circuit.assign_parameters(param_map)
            isa_circuit = pm.run(circuit)
            job         = sampler.run([isa_circuit], shots=1024)

            while job.status() not in ["DONE", "ERROR", "CANCELLED"]:
                time.sleep(2)

            if job.status() != "DONE":
                raise RuntimeError(f"job ended with status: {job.status()}")

            result = job.result()
            data   = result[0].data

            edges_json = read_until_null()
            edges = json.loads(edges_json) if edges_json else []

            decoder = globals()[decoder_name]
            answer  = decoder(data, edges)

            send("OK")
            send(answer)

        except Exception as e:
            send("ERR")
            send(str(e))

if __name__ == "__main__":
    pm, sampler = init()
    send("READY")
    run(pm, sampler)
