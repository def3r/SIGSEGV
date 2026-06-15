import sys
import time
import math
from qiskit.qasm3 import loads
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2
from collections import Counter

def run_qasm(qasm_file: str, theta_0: float, theta_1: float) -> dict:
    service = QiskitRuntimeService()
    backend = service.least_busy(operational=True, simulator=False)

    with open(qasm_file) as f:
        qasm_str = f.read()

    circuit = loads(qasm_str)

    if circuit.parameters:
        param_map = {}
        for p in circuit.parameters:
            if "_theta_0_" in p.name:
                param_map[p] = theta_0
            elif "_theta_1_" in p.name:
                param_map[p] = theta_1
        circuit = circuit.assign_parameters(param_map)

    pm = generate_preset_pass_manager(backend=backend, optimization_level=1)
    isa_circuit = pm.run(circuit)

    sampler = SamplerV2(backend)
    job = sampler.run([isa_circuit], shots=1024)
    print(f"[submit] job id: {job.job_id()}", flush=True)

    while job.status() not in ["DONE", "ERROR", "CANCELLED"]:
        time.sleep(2)

    if job.status() != "DONE":
        print(f"[submit] job ended with status: {job.status()}", file=sys.stderr)
        sys.exit(1)

    result = job.result()
    data = result[0].data
    register_name = list(data.__dict__.keys())[0]
    return getattr(data, register_name).get_counts()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: submit_ibm.py <qasm_file> [theta_0] [theta_1]", file=sys.stderr)
        sys.exit(1)

    theta_0 = float(sys.argv[2]) if len(sys.argv) > 2 else math.pi / 4
    theta_1 = float(sys.argv[3]) if len(sys.argv) > 3 else math.pi / 4

    counts = run_qasm(sys.argv[1], theta_0, theta_1)
    most_frequent = max(counts, key=counts.get)
    print(f"counts:        {counts}")
    print(f"most frequent: {most_frequent}  (decimal: {int(most_frequent, 2)})")

    print("\nTop measurement outcomes (bitstring → count):")
    top = Counter(counts).most_common(4)
    for bitstr, count in top:
        bits = bitstr.replace(" ", "")
        partition_0 = [i for i, b in enumerate(reversed(bits)) if b == "0"]
        partition_1 = [i for i, b in enumerate(reversed(bits)) if b == "1"]
        print(f"  |{bits}> : {count:4d}   S0={partition_0}  S1={partition_1}")
