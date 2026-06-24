# Loop Idiom recogonition
## [LLVM Discussion](https://discourse.llvm.org/t/loop-identification/43655)
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

## [KernelFaRer](https://dl.acm.org/doi/10.1145/3459010)
- BLAS libraries rely heavily on direct use of assembly and thus are not
  portable across platforms, thus the many versions of BLAS

The KernelFaRer’s algorithm can be divided into three phases as follows:

1. Identify candidates that match the target idiom through IR matchers (see
   Section 4.1).
2. Check data dependences and isolate the matched code (see Section 4.2).
3. Replace the idiom with a call to a high-performance library (see Section
   4.3).

Phase 1 uses LLVM’s PatternMatch to identify IR code that matches the target
idiom. The data-dependence analysis in Phase 2 determines if the replacement of
the matched code with a library call is legal. This phase also determines if
code transformations, such as loop distribution or loop invariant code motion,
are needed to make the transformation legal.

## Recogonizing Maxcut Algorithm
- the maxcut implementation that we're trying to identify:
```c
int maxcut(int* edges[2], int ne, int* partition) {
  int cut = 0;
  for (int i = 0; i < ne; i++) {
    int u = edges[i][0];
    int v = edges[i][1];
    if (partition[u] != partition[v]) {
      cut++;
    }
  }
  return cut;
}
```

- LLVM IR with `-O1` optimization:
```llvm
define dso_local i32 @mc(ptr noundef readonly captures(none) %edges, i32 noundef %ne, ptr noundef readonly captures(none) %partition) local_unnamed_addr #0 {
entry:
  %cmp17 = icmp sgt i32 %ne, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext nneg i32 %ne to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %cut.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select, %for.body ]
  ret i32 %cut.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %cut.019 = phi i32 [ 0, %for.body.preheader ], [ %spec.select, %for.body ]
  %arrayidx = getelementptr inbounds nuw ptr, ptr %edges, i64 %indvars.iv
  %0 = load ptr, ptr %arrayidx, align 8, !tbaa !9
  %1 = load i32, ptr %0, align 4, !tbaa !5
  %arrayidx4 = getelementptr inbounds nuw i8, ptr %0, i64 4
  %2 = load i32, ptr %arrayidx4, align 4, !tbaa !5
  %idxprom5 = sext i32 %1 to i64
  %arrayidx6 = getelementptr inbounds i32, ptr %partition, i64 %idxprom5
  %3 = load i32, ptr %arrayidx6, align 4, !tbaa !5
  %idxprom7 = sext i32 %2 to i64
  %arrayidx8 = getelementptr inbounds i32, ptr %partition, i64 %idxprom7
  %4 = load i32, ptr %arrayidx8, align 4, !tbaa !5
  %cmp9.not = icmp ne i32 %3, %4
  %inc = zext i1 %cmp9.not to i32
  %spec.select = add nuw nsw i32 %cut.019, %inc
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !12
}
```

- for loop is optimized, and contains 2 phi-instructions, one for induction var
  `i` and other for the `cut`.
  - a `phi` instruction gets the correct value based on which branch was taken

```llvm
%arrayidx = getelementptr inbounds nuw ptr, ptr %edges, i64 %indvars.iv
%0 = load ptr, ptr %arrayidx, align 8, !tbaa !9
%1 = load i32, ptr %0, align 4, !tbaa !5
```

`getelementptr` (GEP) indexes the _%edges_ in _%indvars.iv_, where each el of
_%edges_ is treated as type _ptr_. Essentially it does _%arrayidx =
%edges\[%indvars.iv\]_. GEP does not deref and gets the data from memory, it
only calculates the ptr. Find more about `inbounds` and `nuw` in the
[docs](https://llvm.org/docs/LangRef.html#getelementptr-instruction).

`load` then derefs the _%arrayidx_, where each el in _%arrayidx_ is treated as
a _ptr_, and reads _8 bytes_ from the memory. More about tis instruction in
[docs](https://llvm.org/docs/LangRef.html#load-instruction)

The next `load` derefs the *%0* and treats each el as an *i32*, and reads 4
bytes from the mem. => we now have `edges[i][0]` in %1


```llvm
%arrayidx4 = getelementptr inbounds nuw i8, ptr %0, i64 4
%2 = load i32, ptr %arrayidx4, align 4, !tbaa !5
```

GEP now takes *%0* which represents the `edges[i]` and treats each el as an
*i8*, ie a byte and indexes 4. ie index 4 bytes from `edges[i]`. The `edges[i]`
points to an array of 2 integers, `edges[i][0]` and `edges[i][1]`. Moving 4
bytes from the `edges[i]` points to `edges[i][1]` because the edges store
integers(4 bytes).

Why use *i8* and then *offset* it instead of directly using the type *i32*?
Good question. The [Archive of the proposal to use *i8* for canonical GEPs with
const offset](https://discourse.llvm.org/t/opaque-pointers-and-i8-geps/58862)
contains the reason behind it. It has something to do with the [LLVM Opaque
Pointer Type](https://llvm.org/docs/OpaquePointers.html).

`load` instruction then derefs the *%arrayidx4* to get the element
`edges[i][1]`

## References
- [Docs Loop Terminology](https://rocm.docs.amd.com/projects/llvm-project/en/latest/LLVM/llvm/html/LoopTerminology.html)
- [Docs LLVM Lang Ref](https://llvm.org/docs/LangRef.html)
