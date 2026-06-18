# TinyIREE: An ML ExecutionEnvironment for EmbeddedSystems from Compilation
- IREE, a unified compiler and runtime stack with the explicit goal to scale
  down machinelearning programs to the smallest footprints for mobile and edge
  devices, while maintaining theability to scale up to larger deployment
  targets
- IREE adopts a compiler-based approach andoptimizes for heterogeneous hardware
  accelerators through the use of the MLIR compilerinfrastructure
- IREE (Intermediate Representation ExecutionEnvironment) was created to
  address this imbal-ance by designing a unified compiler and runtimestack with
  the explicit goal to scale down to thesmallest footprints while retaining its
  ability toscale up to larger deployments (and integrate withhigher-level
  distributed runtimes)
- This focus onmodel portability has driven IREE to effectivelytarget
  bare-metal/embedded CPUs and microcon-trollers
- Traditionally, targeting microcontrollers bypublic ML inference frameworks
  has largely beendone by kernel-based, op-by-op runtimes that arehand-adapted
  and optimized for a small set offrequently used operators.
  - TensorFlow Lite for Microcontrollers (TFLM), aframework specifically
    designed to support modelexecution on microcontrollers
- compiler-basedapproaches have also recently been applied toaccelerators and
  embedded systems
  - One of thosecompilers is Glow [2], which uses a two-phaseintermediate
    representation (IR) to lower a neuralnetwork graph
  - It uses a high-level intermedi-ate representation to apply domain-specific
    op-timizations
  - followed by a lower-level IR,allowing the compiler to apply
    memory-relatedoptimizations
  - optimizations include in-struction scheduling, static memory allocation,and
    copy elimination
- Another compiler is Apache TVM

- IREE is an end-to-end com-piler and runtime framework for model
  executionbased on the MLIR compiler infrastructure
- Itfollows a compiler-based approach that convertsML models into an
  intermediate representationthat allows for analysis and optimization of theML
  model while generating code to target hetero-geneous hardware accelerators
- IREE can take various model represen-tations as inputs, and generate
  executable formatsfor different hardware targets
