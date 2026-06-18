# Triton
- triton primarily for GPU exec
  - Sits b/w cuda (very low level) and PyTorch (very high level)
- `Tiled` prog model, instead of writing logic over individual threads, we write
  it for blocks (block of threads).
  - Compiler handles low level details
  - In CUDA -> thread level, developers write code with thread perspective:
    - Calculate global index from blockIdx, threadIdx, blockDim
    - Handle boundary checks
    - Manage shared memory, synchronization, `warps` yourself
  - In Triton -> block level, developers write code with perspective of `block
    of threads`
  - In CUDA, it is developer's responsibility to build `tiles` per thread, in
    triton tiles are in built lang construct
  - CUDA has better granularity than Triton

# Handling Sparsity
- GPUs are built for dense, regular memory access, Sparsity means irregular mem
  access.
- CUDA being more granular, it gives full control to impl sparse formats like
  `CSR`, `CSC`, `COO`
  - hand tune mem access
  - can use `cuSPARSE`, sparse linear algebra library
  - irregular mem access is the issue
- Triton not good for sparse data
  - Good support for `block sparsity`
  - Limited support for `sparse attention`
  - unstructured sparsity -> "triton struggles here" lol

# Apendix
- `warp` -> a group of 32 threads that exec in `lockstep`
  - basic unit of execution in NVIDIA GPU
  - if-else branches cause issue for warps, it can't split, so it runs the
    branches serially -> performance loss -> called `warp divergence`
- `lockstep` -> all threads in `warp` run the same instruction simlutaneously,
  but on diff data -> `SIMT` (Single Instruction, Multiple Thread)
- `CUDA hierarchy`: 
```
Grid
└── Blocks (you define size, e.g. 256 threads)
    └── Warps (hardware splits block into groups of 32)
        └── Threads (32 per warp)
```
- `tile` -> not about threads, its a chunk of data you're operating on
- `block sparsity` -> skips the block entirely made of `0s`

# Yikes
- [FlashInfer generates CUDA code instead of Triton because Triton still
  underperform CUDA & CUTLASS in many use cases](https://proceedings.mlsys.org/paper_files/paper/2025/file/dbf02b21d77409a2db30e56866a8ab3a-Paper-Conference.pdf)
