# LLVM Discussion
- Instcombine: (instruction combine)
  https://llvm.org/docs/Passes.html#instcombine-combine-redundant-instructions
  - Performs peephole optimizations and canonicalization
    - Canonical form: there are multiple ways of writing things, ex: a+a,
      a*2; a>b, !(a<=b); a canonical form is the form that all different
      representations are finally conveyed in. (ex: all a*2 -> a+a)
    - for peephole opt and guaranteed canonical form, see docs
  - An interesting read on how performance of compiler improved:
    https://developers.redhat.com/articles/2023/12/07/how-single-iteration-instcombine-improves-llvm-compile-time#lessons_learned

- Simplifycfg: https://llvm.org/docs/Passes.html#simplifycfg-simplify-the-cfg
  - performs dead code elimination and BB merging
  - Resources:
    - https://www.reddit.com/r/Compilers/comments/1ijed2b/exploring_llvms_simplifycfg_pass_part_1/

