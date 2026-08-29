[Ref Blog](https://www.stephendiehl.com/posts/mlir_introduction/)

> Interesting stuff below

- E graphs: equlity sautration is a tech to build optimizing compilers
- basic blocks in mlir can take args(!) unlike llvm.


> Defs

- Operations: The basic unit of work in MLIR
- Range: a way to group operations together via `{` and `}`
       :  (can take args)


> dialects

1. `llvm` : lowest level dialect in the MLIR hierarchy
2. `scf` and `cf` : structured control flow and control flow
  - `cf.branch` : branch to a bb
  - `cf.cond_br` : cond branch to a bb
  - `cf.switch` : switch to bb
  - `scf.if` : if statement
  - `scf` ops are lowered to `cf` via `convert-scf-to-cf`
3. `arith` : arithmatic ops
  - Integer airth : `addi`, `subi`, `muli`, `divsi` (signed), `divui` (unsigned)
  - fp arith : `addf`, `subf`, `mulf`, `divf`
  - cmp : `cmpi` (intiger), `cmpf` (float)
  - conversions : `extsi` (sign extend), `extui` (zero extend), `trunci`,
    `fptoui`, `fptosi`, `uitofp`, `sitofp`
  - bitwise ops : `andi`, `ori`, `xori`
4. `math` : complex math ops:
  - Trig : `sin`, `cos`, `tan`
  - Exp : `exp`, `exp2`, `log`, `log2`, `log10`
  - Power : `pow`, `sqrt`
5. `index` : dialect to handle index computations, the index type is
  platform-specific sized integer used for addressing and loop indction vars.
  - `constant` : creates and index const
  - `add`, `sub`, `mul` : ops two indices
  - `cmp` : cmp two indices
  - `divs`, `rems` : signed div and rem of two indices
  - useful when working with tensors and mem layouts

> Interesting mlir-opt passes

- `convert-func-to-llvm`
- `convert-scf-to-cf`
- `convert-omp-to-llvm`
- `-convert-to-llvm` : converts everthing that can be converted to llvm from
  mlir

- use `--pass-pipeline="builtin.module(pass1,pass2)"` to run pass1 and pass2
  sequentially in one group

> debugging

- `--mlir-print-ir-after-all` prints ir after each pass. Similar are:
  `--mlir-print-ir-after-change`, `-mmlir-print-ir-after-failure`
- While using print-ir flags, using `--mlir-print-ir-tree-dir` writes the IR to
  to files in a dir instead of stdout.


> compiling the `simple.mlir` to `simplified.mlir`

```bash
mlir-opt simple.mlir \
  --convert-func-to-llvm \
  --convert-math-to-llvm \
  --convert-index-to-llvm \
  --convert-scf-to-cf \
  --convert-cf-to-llvm \
  --convert-arith-to-llvm \
  --reconcile-unrealized-casts \
  -o simplified.mlir
```

> running main func directly via mlir

```bash
mlir-runner -e main -entry-point-result=i32 simplified.mlir
```

> creating `.so` files from mlir

```bash
mlir-translate simple_opt.mlir -mlir-to-llvmir -o simple.ll
llc -filetype=obj --relocation-model=pic simple.ll -o simple.o
clang -shared -fPIC simple.o -o libsimple.so
```

- where the dissassembly of simplified.o can be found by `objdump -d simplified.o`


