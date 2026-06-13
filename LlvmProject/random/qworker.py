import sys
import time
from qiskit.qasm3 import loads
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager
from qiskit_ibm_runtime import QiskitRuntimeService, SamplerV2

# decoders

def add(data):
    register = list(data.__dict__.keys())[0]
    counts = getattr(data, register).get_counts()
    most_frequent = max(counts, key=counts.get)
    return str(int(most_frequent, 2))

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
    pm = generate_preset_pass_manager(backend=backend, optimization_level=1)
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
            circuit     = loads(qasm_str)
            isa_circuit = pm.run(circuit)
            job         = sampler.run([isa_circuit], shots=1024)

            while job.status() not in ["DONE", "ERROR", "CANCELLED"]:
                time.sleep(2)

            if job.status() != "DONE":
                raise RuntimeError(f"job ended with status: {job.status()}")

            result = job.result()
            data   = result[0].data

            decoder = globals()[decoder_name]
            answer  = decoder(data)

            send("OK")
            send(answer)

        except Exception as e:
            send("ERR")
            send(str(e))

if __name__ == "__main__":
    pm, sampler = init()
    send("READY")
    run(pm, sampler)
