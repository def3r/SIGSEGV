[Blog](https://www.stephendiehl.com/posts/mlir_memory/)

> Memory management in MLIR

3 primary ways to represent data in MLIR:

1. Tensors for immutable, abstract ops (pure mathematical objects)
2. MemRefs for concrete mem bufs
3. LLVM-level constructs for fine grain control (low level mem ops)


> Tensors

- immutable, high-level data structs without explicit mem locs


> Memrefs

- represents a pointer to a region of mem with additional metadata
  about its struct
- Components of memref: a. *metadata*, b. *data*
  - Allocated pointer : points to data buf (used for dealloc)
  - Aligned pointer : points to properly aligned data
  - Offset : dist b/w aligned pointer start & first accessible el
  - Shape : arr of ints describing dims
  - Strides : arr of ints describing mem layout

> NumPy arr mem layout

1. data buf : contiguous mem block holding data
2. Shape : tuple describing arr dims
3. strids : tuple describing steps (in bytes) to move in each dims
4. Dtype : data type of els

To find the mem addr of `[i, j]` el in matrix:
```
address = base_address + (i * strides[0] + j * strides[1])
```

row major strided layout for `memref<3x4xf32>` is same as:
`memref<3x4xf32m strided<[4, 1], offset: 0>>`


> Integrating MLIR with python

Two main approaches:
1. Compiling to shared lib and loading it
2. JIT compiling the MLIR code at runtime


> Loading Pre-compiled MLIR Code

refer `./mlir2so.sh` and `useso.py`


> JIT compiling MLIR @ runtime

using `llvmlite` (python bindings for LLVM)

refer `./usejit.py`


> LLVM dialect

- arrays fized size known at compile time and dont carry any metadata
  about their structure


> vector dialect

- low level abstraction for SIMD ops
- it provides ops that may not be directly correspond to h/w cap but
  LLVM handles the lowering process.


> Bufferization
- process of converting ops with tensor semantics to ops with memref
  semantics
- ie proc of converting computations on the mathematical tensor
  construct to computations on physical mem buffers
- `bufferization.to_memref` - converts a tensor to memref
- `bufferization.to_tensor` - converts a memref to a tensor
- pass `one-shot-bufferize` converts all tensor ops into memref ops,
  but it does not bufferize input and output of a function (called
  func boundaries). To bufferize func boundaries, we use
  `bufferize-function-boundaries` flag

```bash
mlir-opt -one-shot-bufferize="bufferize-function-boundaries"
```


> C-compatible wrappers

- To call a mlir func from C, we need to emit C-compatible wrapper:
  `llvm.emit_c_interface` attribute
- This is done because bufferization of a function unrolls all the
  memref fields into a single args. This isn't desirable when we
  want to call the func from C, where we pass the memrefs as void
  ptrs to structs
- emits a wrapper func `@_mlir_ciface_{fname}`
- refer `array_add_opt.mlir` for ex func
