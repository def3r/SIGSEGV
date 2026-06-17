from qiskit_ibm_runtime import QiskitRuntimeService
from qiskit.qasm3 import loads, dumps
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager

service = QiskitRuntimeService()
backend = service.least_busy(operational=True, simulator=False)
print(f"selected: {backend.name}")

with open("add_71_11_circuit.qasm") as f:
    circuit = loads(f.read())

for opt_level in [1, 3]:
    pm  = generate_preset_pass_manager(backend=backend, optimization_level=opt_level)
    isa = pm.run(circuit)

    print(f"\n--- optimization_level={opt_level} ---")
    print(f"depth:  {isa.depth()}")
    print(f"qubits: {isa.num_qubits}")
    print(f"gates:  {isa.count_ops()}")

    with open(f"mul_7_11_isa_O{opt_level}.qasm", "w") as f:
        f.write(dumps(isa))

print("\nwritten: mul_7_11_isa_O1.qasm and mul_7_11_isa_O3.qasm")
