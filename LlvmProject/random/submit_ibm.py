import sys
import time
from qiskit.qasm3 import loads
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2

def run_quantum_add(qasm_file: str) -> int:
    service = QiskitRuntimeService()
    backend = service.least_busy(operational=True, simulator=False)

    with open(qasm_file) as f:
        qasm_str = f.read()

    circuit = loads(qasm_str)

    # transpile to IBM ISA - converts u3 etc to native gate set
    pm = generate_preset_pass_manager(backend=backend, optimization_level=1)
    isa_circuit = pm.run(circuit)

    sampler = SamplerV2(backend)
    job = sampler.run([isa_circuit], shots=1024)

    while job.status() not in ["DONE", "ERROR", "CANCELLED"]:
        time.sleep(2)

    if job.status() != "DONE":
        sys.exit(1)

    result = job.result()
    print(result)
    print("SKIBIDI")
    print(result[0].data.__dict__.keys())
    print("SKIBIDI")
    data = result[0].data
    print(data)
    register_name = list(data.__dict__.keys())[0]
    counts = getattr(data, register_name).get_counts()

    most_frequent = max(counts, key=counts.get)
    return int(most_frequent, 2)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: submit_ibm.py <qasm_file>", file=sys.stderr)
        sys.exit(1)

    result = run_quantum_add(sys.argv[1])
    print(result)
