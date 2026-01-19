*https://www.stephendiehl.com/posts/mlir_introduction/*<br>

mlir to llvm dialect:
```
mlir-opt example.mlir \
  --convert-func-to-llvm \
  --convert-math-to-llvm \
  --convert-index-to-llvm \
  --convert-scf-to-cf \
  --convert-cf-to-llvm \
  --convert-arith-to-llvm \
  --reconcile-unrealized-casts \
  -o example_opt.mlir
```
