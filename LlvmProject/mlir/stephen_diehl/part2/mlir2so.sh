# Compile MLIR to shared library
mlir-opt array_add.mlir \
  --convert-tensor-to-linalg \
  --convert-linalg-to-loops \
  --convert-scf-to-cf \
  --convert-cf-to-llvm \
  --convert-math-to-llvm \
  --convert-arith-to-llvm \
  --convert-func-to-llvm \
  --convert-index-to-llvm \
  --finalize-memref-to-llvm \
  --reconcile-unrealized-casts \
  -o array_add_opt.mlir

mlir-translate --mlir-to-llvmir array_add_opt.mlir > array_add.ll
llc -filetype=obj array_add.ll -o array_add.o
clang -shared -fPIC array_add.o -o libarray_add.dylib
