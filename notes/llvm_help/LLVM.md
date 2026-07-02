# LLVM HELP

## Generic
- llvm source code for different archs lives at: `llvm/lib/Target/`

## ISSUES
- `Implement clz using floating point math if clz instruction is not available` [#161746](https://github.com/llvm/llvm-project/issues/161746)
  - `clz` instruction -> Count Leading Zeroes [src](https://stackoverflow.com/questions/9353973/implementation-of-builtin-clz)
    - `llvm.ctlz` -> same thing, but llvm intrinsic
  - `Zbb` -> RISC V bit manipulation ISA extension (one of `a`, `b`, `c` and `s` extension) [src](https://www.ece.lsu.edu/ee4720/doc/riscv-bitmanip-1.0.0.pdf)
  - `llvm-mca` [LLVM Machine Code Analyzer](https://llvm.org/docs/CommandGuide/llvm-mca.html)
  - `SelectionDAG`: [src: Revisit](https://llvm.org/devmtg/2024-10/slides/tutorial/MacLean-Fargnoli-ABeginnersGuide-to-SelectionDAG.pdf)
    - Framework for instruction selection
    - Part of `llc` (LLVM Static Compiler)
    - Each `SelectionDAG` represents a single basic block
  - `EVT` -> Extended Value Type
  - `MVT` -> Machine Value Type
  - `SetCC` -> Set Conditional Code
  - `SDValue` -> Selection DAG Value
  - `ISel` -> Instruction Selection
  - `gpr` -> General Purpose Registers

  - Opcodes:
    - `CTLZ` -> count leading zeroes
    - `CTLZ_ZERO_UNDEF` -> count leading zeroes but give undef behavior on zero

  - X86:
  - `SSE2` -> Streaming SIMD Extension 2 == Instruction Set for Intel SIMD
  - `SSSE3` -> Suplemental Streaming SIMD Extension 3 (4th increment of the `SSE` tech)
  - `XMM registers`
    - 128-bit registers introduced with SSE for packed float/int computations
    - In 64-bit mode for scalar floating-point math
  - `AVX registers`
    - `Advanced Vector Extension` registers for SIMD
    - `YMM` uses 256-bit regs, `ZMM` uses 512 bit regs

  - X86 ISA:
  - `CVTSI2SD` -> Convert Doubleword Integer to Scalar Double-Precision Floating Point Values
  - `CVTDQ2PD` -> Convert Packed Doubleword Integers to Packed Double Precision Floating-PointValues

  - Tests:
  - `Check` lines

- `[GlobalISel] Port known-bit and known-sign-bits for all operations` [#150515](https://github.com/llvm/llvm-project/issues/150515)
  - importing `FSHL` and `FSHR`
  - `GISel` -> Global Instruction Selection
  - `Exhaustive tesiting` -> Tests all possible values in the input set

  - Opcodes:
    - `zext` -> Zero extend

## Commands
### Build
1. From inside the `build` dir:
```bash
mkdir build && cd build
cmake -G "Ninja" \
 -DCMAKE_INSTALL_PREFIX=../install \
 -DCMAKE_BUILD_TYPE="Debug" \
 -DLLVM_TARGETS_TO_BUILD="X86;AArch64" \
 -DBUILD_SHARED_LIBS=1 \
 -DCMAKE_CXX_FLAGS=" -ggdb3 -gdwarf-4 " \
 -DLLVM_ENABLE_BINDINGS=OFF  \
 ../llvm
cd ..
cmake --build build
```

2. `clang-format`: `git clang-format HEAD~1`
3. `llvm-mca` Analsis: `build/bin/llvm-mca -mtriple=x86_64-unknown-unknown
   -mcpu=core2 -timeline testv4_v2.asm > testv4_v2.txt`

### Tests
1. build all tests (including tools): `ninja check-all -j8`
2. Tests update script:
```bash
cmake --build build -- llc # First make sure binaries are up to date

# Update scripts may differ
llvm/utils/update_llc_test_checks.py \
 --llc-binary build/bin/llc \
 llvm/test/CodeGen/X86/vector-lzcnt-128.ll # This is the file to be updated
```
3. Run Single test file: `build/bin/llvm-lit build/test/CodeGen/X86/combine-srl.ll -v`
4. Run Target specific tests: `cmake --build build --target check-llvm-codegen-x86`
5. Run Regression tests only: `ninja check-llvm`

#### MIR Tests
1. Find a relevant MIR file, copy its contents, update tests
2. Add Something like: `# RUN: llc -mtriple aarch64
   -passes="print<gisel-value-tracking>" %s -o - 2>&1 | FileCheck %s`, at the
   top of the file (This will be interpreted as a cmd by update script)
3. Run the update script ex: `llvm/utils/update_givaluetracking_test_checks.py
   --llc-binary=build/bin/llc
   llvm/test/CodeGen/AArch64/GlobalISel/knownbits-fshl-fshr.mir`
4. Tests add relevant `Check lines`

#### Unit Tests
1. build specific test: `cmake --build build --target SupportTests`
2. Run the test: `build/unittests/Support/SupportTests --gtest_filter="*KnownBitsTest*"`

## SUGGESTIONS
1. [puttu.net](https://www.puttu.net/)
  - [Building LLVM](https://github.com/vaivaswatha/misc/blob/master/LLVMBuild.md)

## DOCS:
1. [Test Suite Wiki](https://llvm.org/docs/TestSuiteGuide.html)
2. [MIR Tests Wiki](https://llvm.org/docs/MIRLangRef.html#introduction)

## Articles
1. [Legalizations in LLVM Backend](https://myhsu.xyz/llvm-codegen-legalization/)
  - [Funnel / Rotate Shifts](https://myhsu.xyz/llvm-codegen-legalization/#:~:text=What%20are%20funnel%20%2F%20rotate%20shifts)
2. [Undef and Poison](https://github.com/thaliaarchi/compiler-notes/blob/main/llvm/devmtg_2020-10/undef_and_poison.md?plain=1)

## `clang` to llvmir
- Emit llvm ir: `clang -emit-llvm -S input.c`
- flags:
  - get var names instead of ssa values: `-fno-discard-value-names`
- passes:
  - Get rid of alloca: `opt -passes="mem2reg" -S input.ll -o output.ll`

## Issues to take
- https://github.com/llvm/llvm-project/issues/174584
- stale: https://github.com/llvm/llvm-project/pull/195405 ::
  https://github.com/llvm/llvm-project/issues/189583
- Too late: https://github.com/llvm/llvm-project/issues/195462
- The rem func: https://github.com/llvm/llvm-project/issues/116695
  examples: https://github.com/llvm/llvm-project/pull/120030
            https://github.com/llvm/llvm-project/pull/120903
    (stale) https://github.com/llvm/llvm-project/pull/167147
- Good Read: https://github.com/llvm/llvm-project/issues/174214
- Must Read: https://dl.acm.org/doi/pdf/10.1145/3808250

### Come back after sometime
- https://github.com/llvm/llvm-project/issues/189694
- [ ] *`assigned`* instcombine one: https://github.com/llvm/llvm-project/issues/165306
- MIRSample crash https://github.com/llvm/llvm-project/issues/189764
- https://github.com/llvm/llvm-project/issues/189479
- IT@VJTI: https://github.com/llvm/llvm-project/issues/164161
- Not any contenders: https://github.com/llvm/llvm-project/issues/161642
- Maybe look at it in August: https://github.com/llvm/llvm-project/issues/156883
- Maybe mid July: https://github.com/llvm/llvm-project/issues/139786

OR

- https://github.com/llvm/llvm-project/issues?q=is%3Aissue%20state%3Aopen%20label%3A%22good%20first%20issue%22&page=4
- https://github.com/llvm/llvm-project/issues?q=is%3Aissue%20state%3Aopen%20label%3A%22good%20first%20issue%22%20no%3Aassignee%20sort%3Aupdated-desc

vim: foldmethod=indent
