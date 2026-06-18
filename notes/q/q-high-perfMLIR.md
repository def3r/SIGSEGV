# Towards a high-performance AI compiler with upstream MLIR
-  We demonstrate this flow witha proof-of-concept MLIR project that uses input
   IR in Linalg-on-Tensor from TensorFlow and PyTorch,performs cache-level
   optimizations and lowering to micro-kernels for efficient vectorization,
   achieving over90% of the performance of ninja-written equivalent programs.
- This work3 builds on the success of TPP’s state-of-the-art performance of
  various algorithms on a varietyof CPUs by bringing a set of high-level linear
  algebra compiler passes to automatically choose the correct TPPoperations, in
  the right order, with the suitable flags, including packing tensors and
  adjusting iteration spacesfor optimal traversal and full utilization of
  hardware resources.
