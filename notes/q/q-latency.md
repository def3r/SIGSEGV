https://arxiv.org/pdf/2602.20204

# Analyzing Latency Hiding and Parallelism in an MLIR-based AIKernel Compiler
- This paper reports a benchmark methodology and corresponding results for
  three compiler-controlled mechanisms in an MLIR-based compilation pipeline:
  vectorization (Vec), multi-threading(MT) across hardware contexts, and double
  buffering (DB) using ping–pong scratchpad buffers to overlapDMA transfers
  with compute.
- What is an NPU?
  - A Neural Processing Unit (NPU) is a specialized microprocessor designed
    specifically to accelerate machine learning tasks
- an NPU is optimized for the heavy mathematical lifting required by
  AI—specifically tensor and vector operations
- characteristics of NPUs mentioned in the paper include:
  - Hierarchical Memory
  - Explicit Data Movement
  - Vector Contexts
- If you want to train a new AI model from scratch on billions of words, you
  use a GPU because of its raw power and mature software. But if you want your
  phone to run that model to blur your background in a video call without
  burning your hand or killing the battery, you use an NPU.

## Introduction
- On edge NPUs,performance is shaped by hierarchical memory, explicit
  DMA-managed transfers, and the need to schedulework so compute stays busy
  while transfers are in flight.
- quantify the impact of vectorization (Vec), multi-threading (MT), and
  doublebuffering (DB)
  - Vec exploits data-level parallelism
  - MT exploits loop- and region-level parallelism bydistributing independent
    tiles across hardware contexts
  - DB reduces stall time by overlapping memorytransfers with compute
- In this ladder, Vec isolates SIMD-style lowering, Vec+MT quantifies
  incremental thread-level speedup, andVec+MT+DB evaluates whether an explicit
  latency-hiding schedule provides additional improvement onceVec and MT are
  already in place

## Implementation Details: Multi-threading and Double Buffering
- design principle: keep the intent expressed in structuredIR for as long as possible, and lower to runtime constructs only after the compiler has imposed a schedule.

### Multi-threading (MT)
