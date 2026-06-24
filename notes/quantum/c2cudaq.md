> mapping classical 2 cudaq kernels, inspired by C2Q

ops to support:
- Arithmetic: ADD, SUB, MUL
- QUBO -> QAOA/VQE: Maxcut, MIS, VC, Clique, KColor, TSP
- Grover Oracle: Factor, (also MIS)

1. Each graph problem goes through: QUBO -> Ising -> QAOA circuit
  - `QUBO`: Quadratic Uncostrained Binary Optimization. To solve any
    combinatorial problem on a quantum device, we need to *translate* the
    problem into QUBO form. This concept has nothing to do with quantum.
  - `QAOA`: Quantum Approximate Optimization Algorithm. This is used to solve
    combinatoria optimizations on NISQ devices.
    - Ofc Quantum circuits consist of quantum gates, but we can also describe a
      quantum circuit in terms of `Hamiltonian`.
    - The cost Hamiltonian is expressed as the sum of *Pauli-Z operators*. To
      do this, we need to map the QUBO to the Ising model.
    - Algorithm for `QAOA`:
      1. Define a cost Hamiltonian such that its ground state encodes the
      solution to the optimization problem.
      2. Define a mixer Hamiltonian
      3. Construct the circuits.
      4. Choose a parameter and build the circuit consisting of repeated
      application of the cost and mixer layers.
      5. Prepare an initial state, and use classical techniques to optimize the
      parameters.
      6. After the circuit has been optimized, measurements of the output state
      reveal approximate solutions to the optimization problem.
  - `Ising` model: It is a physical system based on spins {+1, -1}. This fits
    perfectly with the hardware model of the qubits. Thus this makes the
    Hamiltonian "hardware friendly".

### References
- https://pennylane.ai/demos/tutorial_QUBO
- https://pennylane.ai/demos/tutorial_qaoa_intro
- https://quantumcomputinginc.com/learn/lessons/ising-models
