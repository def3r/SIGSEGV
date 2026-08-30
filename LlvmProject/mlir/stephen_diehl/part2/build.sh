mlir-opt array_add.mlir \
  -finalize-memref-to-llvm \
  -convert-scf-to-cf \
  -convert-cf-to-llvm \
  -convert-arith-to-llvm \
  -convert-func-to-llvm \
  --reconcile-unrealized-casts \
  -o array_add_opt.mlir

mlir-translate array_add_opt.mlir \
  -mlir-to-llvmir \
  -o array_add_opt.ll

llc -filetype=obj --relocation-model=pic array_add_opt.ll -o arary_add.o
clang -shared -fPIC arary_add.o -o array_add.so
