 ▐▛███▜▌   Claude Code v2.1.195
▝▜█████▛▘  Sonnet 4.6 · Claude Pro
  ▘▘ ▝▝    ~/def3r/SIGSEGV/LlvmProject/loopanalysis/test/maxcut


❯ I have written a pass to identify maxcut algorithm. The algorithm impl in C++ 
  is in @test/maxcut_actual.cpp, extracted the llvm IR is in                    
  @test/clean_target.ll and then opt it to @test/clean5.ll i used this:  clang  
  -S -emit-llvm -O0 -fno-inline -Xclang -disable-O0-optnone                     
  -fno-discard-value-names maxcut_actual.cpp                                    
   to make the IR, and then: llvm-extract                                       
  -func=_Z6actualSt6vectorIiSaIiEES_ISt4pairIiiESaIS3_EE maxcut_actual.ll -S -o 
  clean_target.ll                                                               
   and then opt it:  opt -passes="sroa,mem2reg,loop-simplify,lcssa,indvars,simp 
  lifycfg,instcombine,simplifycfg,instcombine" clean_target.ll -S -o clean5.ll  
  can you write tests for the maxcut algo identification pass? Do it in the     
  test/maxcut/ dir. The tests should correctly identify the diffferent types    
  (but similar to maxcut_actual.cpp impl) of maxcut and not identify types that 
  are similar but are not actually maxcut. What im saying is no False           
  Positives. write some tests. Also point out some flaws on my pass, be it on   
  pattern matching or something else wrong with my pass, be critical because    
  this is my first time writing an LLVM pass to identify an algorithm. To be    
  sure, use the above sequence of optimizations for the IR because thats what   
  my pass will work on. Before writing test code, evaluate and tell me whats    
  your testing plan.                                                            
  ⎿  Read test/maxcut_actual.cpp (81 lines)
  ⎿  Read test/clean_target.ll (452 lines)
  ⎿  Read test/clean5.ll (260 lines)

● Let me explore the project structure to understand the existing pass before
  planning tests.

  Searched for 1 pattern, read 2 files, listed 1 directory (ctrl+o to expand)

● Now let me look at the CMakeLists, readme, and some existing test IR files to
  understand the build setup and test conventions.

  Read 3 files, ran 1 shell command (ctrl+o to expand)

● Let me check how the pass is invoked and look at the existing test IR to
  understand patterns:

  Listed 1 directory, ran 1 shell command (ctrl+o to expand)

● Now I have a complete picture of both passes. Let me analyze everything before
  writing the tests.

  Ran 1 shell command (ctrl+o to expand)

● Now I have everything I need to give you a thorough critique and write the
  tests. Let me write it all out:

  ---
  Testing Plan (before writing code)

  What I'm testing: maxcut-cpp-pass in maxcut_cpp_pass.cpp — not MinPass in
  pass.cpp.

  Test taxonomy:

  ┌──────────────────────┬──────────────────────────────────────────────────┐
  │       Category       │                     Meaning                      │
  ├──────────────────────┼──────────────────────────────────────────────────┤
  │ True Positive (TP)   │ IS maxcut → pass SHOULD detect → verify it does  │
  ├──────────────────────┼──────────────────────────────────────────────────┤
  │ True Negative (TN)   │ NOT maxcut → pass SHOULD reject → verify it does │
  ├──────────────────────┼──────────────────────────────────────────────────┤
  │ False Positive       │ NOT maxcut → pass INCORRECTLY detects → exposes  │
  │ (FP-bug)             │ bug                                              │
  ├──────────────────────┼──────────────────────────────────────────────────┤
  │ False Negative       │ IS maxcut → pass INCORRECTLY misses → exposes    │
  │ (FN-bug)             │ limitation                                       │
  └──────────────────────┴──────────────────────────────────────────────────┘

  7 test cases:
  1. tp_basic.cpp — TP: structural clone, different names
  2. tp_val_only.cpp — FN-bug: value-only (no vector copy in update → select
  form → BFS finds no MaxUpdatePhi)
  3. tn_min_cut.cpp — TN: min-cut (non-zero init → Phase 2.1 rejects; icmp slt →
  Phase 2.4 rejects)
  4. tn_sum.cpp — TN: sum all cuts (outer loop adds, not selects max → no icmp 
  sgt → Phase 2.4 rejects)
  5. tn_directed.cpp — FP-bug: directed cut (AND condition → Step 1.9 does
  continue not return nullopt → slips through)
  6. tn_both_in.cpp — FP-bug: edge-inside-S count (AND condition → same bug)
  7. fn_long_edges.cpp — FN-bug: pair<long long, long long> edges (GEP stride 16
  ≠ hardcoded 8 → Step 1.7 rejects)

  Now writing the files:

● Write(tp_basic.cpp)
  ⎿  Wrote 53 lines to tp_basic.cpp
       1 // tp_basic.cpp — TRUE POSITIVE
       2 // Same algorithm as the reference (maxcut_actual.cpp), differen
         t function/variable names.
       4 //
       5 // What's the same as reference:
       6 //   - Outer loop: iterate all subsets, track maximum cut
       7 //   - Inner loop: iterate edges, two std::find calls, XOR condi
         tion, add-1 accumulator
       9 //   - icmp sgt for max update, phi merge after branch
      10 //
     … +48 lines (ctrl+o to expand)

● Write(tp_val_only.cpp)
  ⎿  Wrote 48 lines to tp_val_only.cpp
       1 // tp_val_only.cpp — FALSE NEGATIVE (pass limitation)
         gnment in the
       3 // outer update block. This causes instcombine/simplifycfg to fo
       4 // if-then block into a `select` instruction rather than leaving
          a phi merge.
       6 // Expected: SHOULD be detected as MaxCut but IS NOT.
       7 // Failure point: Phase 2.4 BFS looks for a PHINode with incomin
       8 //   (CutLCSSA, MaxPhi). When the update folds to:
       9 //     %best.new = select i1 %cmp, i32 %cut, i32 %best
         t CutLCSSA
     … +49 lines (ctrl+o to expand)

● Write(tn_min_cut.cpp)
  ⎿  Wrote 50 lines to tn_min_cut.cpp
       1 // tn_min_cut.cpp — TRUE NEGATIVE
         ical to the
       3 // reference MaxCut except the outer comparison tracks the MINIM
       4 // count, not the maximum.
       5 //
       7 //   Phase 2.1 — MaxPhi init check: initialized to edges.size() 
         (not 0)
         lt
       9 //
          (which would
     … +48 lines (ctrl+o to expand)

● Write(tn_sum.cpp)
  ⎿  Wrote 43 lines to tn_sum.cpp
       1 // tn_sum.cpp — TRUE NEGATIVE
         n tracking the
       3 // maximum. The outer accumulator is ADD-based, not max-based.
       4 //
       6 //   The outer loop has no `icmp sgt(inner_acc, outer_acc)` — th
         e outer
          between the
       8 //   inner cut value and the running sum. Phase 2.4 scans all ou
         ter-loop
      10 //
     … +40 lines (ctrl+o to expand)

● Write(tn_directed.cpp)
  ⎿  Wrote 59 lines to tn_directed.cpp
       1 // tn_directed.cpp — FALSE POSITIVE (pass bug!)
         on only
       3 // (a∈S and b∉S), using an AND condition instead of XOR.
       4 //
         is a different
       6 // problem (max directed cut / max s-t flow objective).
       8 // Expected: should NOT be detected. BUT the current pass HAS A 
         BUG:
       9 //   Step 1.9 checks if the condition is XOR via:
          }
     … +56 lines (ctrl+o to expand)

● Write(tn_both_in.cpp)
  ⎿  Wrote 58 lines to tn_both_in.cpp
       1 // tn_both_in.cpp — FALSE POSITIVE (pass bug!)
         the number of
       3 // "internal edges" (a measure of subgraph density). This is the
       4 // cut-edge counting. NOT a MaxCut algorithm.
       6 // Expected: should NOT be detected. BUT the current pass HAS TH
         E SAME BUG
       8 //   Step 1.9 uses `continue` instead of `return std::nullopt` f
         or non-XOR.
         e_b`.
          function
     … +59 lines (ctrl+o to expand)

● Write(fn_long_edges.cpp)
  ⎿  Wrote 63 lines to fn_long_edges.cpp
       1 // fn_long_edges.cpp — FALSE NEGATIVE (pass limitation: hardcode
       2 // MaxCut algorithm identical to the reference, but with edge en
         dpoints as
         the graph
       4 // structure is the same, just using 64-bit node IDs.
       6 // Expected: SHOULD be detected as MaxCut but IS NOT.
       7 // Failure point: Step 1.7 checks the inner latch GEP stride:
       9 // pair<int,int> has sizeof = 8, but pair<long long, long long> 
         has sizeof = 16.
         s it.
     … +69 lines (ctrl+o to expand)

● Now write the generation + verification script:

● Write(gen_ir.sh)
  ⎿  Wrote 123 lines to gen_ir.sh
       1 #!/usr/bin/env bash
         s the reference,
       3 # then run maxcut-cpp-pass and report whether each case was dete
       4 #
       5 # Pipeline (per the project README):
         ptnone
       7 #            -fno-discard-value-names <src>
       9 #   3. opt -passes="sroa,mem2reg,loop-simplify,lcssa,indvars,sim
         plifycfg,
         x.ll -S -o <src>_opt.ll
     … +137 lines (ctrl+o to expand)

  Ran 1 shell command (ctrl+o to expand)

● All 7 tests run and confirm the analysis. Every prediction was correct. Now
  let me present the full critique and save a memory entry.
● Write(~/def3r/.claude/projects/-home-def3r-def3r-SIGSEGV/memory/project_loopan
       alysis.md)
  ⎿  Wrote 25 lines to ../../../../../.claude/projects/-home-def3r-def3r-SI
     GSEGV/memory/project_loopanalysis.md
       1 ---
       2 name: project-loopanalysis
       3 description: LLVM pass project (loopanalysis) — identifies MaxCu
       4 metadata:
       5   type: project
       6 ---
       7 
       8 Project is an LLVM out-of-tree pass plugin at `/home/def3r/def3r
       9 
      10 Two passes in the same `.so`:
     … +22 lines (ctrl+o to expand)

  Read 1 file (ctrl+o to expand)

● Write(~/def3r/.claude/projects/-home-def3r-def3r-SIGSEGV/memory/MEMORY.md)
  ⎿  Wrote 3 lines to ../../../../../.claude/projects/-home-def3r-def3r-SIG
     SEGV/memory/MEMORY.md
      1 # Memory Index
      2 
      3 - [loopanalysis project](project_loopanalysis.md) — LLVM out-of-t
        ree pass plugin for MaxCut algorithm identification; two passes (
        C-style and C++ STL-style); test suite at test/maxcut/

● All 7 tests run and all predictions held. Here's the full report.

  ---
  Testing Plan (executed)

  Pipeline per your spec, then opt -load-pass-plugin build/MinPass.so 
  -passes=maxcut-cpp-pass -disable-output on the result.

  ┌───────────────────┬───────────────┬───────────────────────────────────┐
  │       File        │   Category    │              Result               │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tp_basic.cpp      │ True Positive │ ✅ PASS — detected                │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tn_min_cut.cpp    │ True Negative │ ✅ PASS — rejected                │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tn_sum.cpp        │ True Negative │ ✅ PASS — rejected                │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tn_directed.cpp   │ FP bug        │ 🐛 BUG — false positive confirmed │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tn_both_in.cpp    │ FP bug        │ 🐛 BUG — false positive confirmed │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ tp_val_only.cpp   │ FN bug        │ 🐛 BUG — false negative confirmed │
  ├───────────────────┼───────────────┼───────────────────────────────────┤
  │ fn_long_edges.cpp │ FN limitation │ 🐛 BUG — false negative confirmed │
  └───────────────────┴───────────────┴───────────────────────────────────┘

  Run with bash test/maxcut/gen_ir.sh from the project root to reproduce.

  ---
  Flaws in maxcut_cpp_pass.cpp

  Critical: False Positives (wrong algorithm identified as MaxCut)

  1. Step 1.9: non-blocking XOR gate check (maxcut_cpp_pass.cpp:311)

  This is the most serious correctness bug. When the condition is not XOR, the
  code does:
  if (XorInst->getOpcode() != Instruction::Xor) {
      continue;  // ← skips, does NOT return nullopt
  }
  After the loop exits with no rejection, matchScoringLoop returns a valid
  ScoringLoopMatch. Any algorithm that has the same structural shell (2 finds,
  accumulator, same outer loop) but uses an AND condition instead of XOR — like
  directed cut (a_in && !b_in) or internal-edge counting (a_in && b_in) — gets
  incorrectly flagged as MaxCut. The fix is: require that at least one
  predecessor's branch is a valid XOR, and return std::nullopt if none are.

  2. isValidCmp returns true even when VecEnd is not vector::end()
  (maxcut_cpp_pass.cpp:320-353)

  The lambda always returns true at the end, meaning the final check if (auto* 
  CB = dyn_cast<CallBase>(VecEnd)) { ... validFinds[idx] = true; } is dead for
  the validation path. If VecEnd is not a vector::end() call, validFinds[idx]
  stays false, but the function still returns true, so the XOR operand is
  "validated" even when it isn't. This is lower impact than bug #1 since it only
  fires when there IS a XOR — but it means the XOR operand check is incomplete.

  ---
  Serious: False Negatives (valid MaxCut missed)

  3. Hardcoded GEP stride = 8 in Step 1.7 (maxcut_cpp_pass.cpp:375)

  if (!Idx || !Idx->equalsInt(8)) continue;
  sizeof(pair<int,int>) == 8 on x86-64, but this hardcodes that forever.
  pair<long long, long long> has stride 16, pair<short, short> has stride 4 —
  both valid MaxCut formulations are missed. The stride should be derived from
  the actual pair element type, or at minimum asserted against the type rather
  than a magic constant.

  4. Hardcoded GEP stride = 24 in Step 2.5 (maxcut_cpp_pass.cpp:516)

  if (!Idx || !Idx->equalsInt(24)) continue;
  sizeof(vector<int>) == 24 on x86-64 (three pointers). Same issue: architecture
  or ABI changes break this. Should be derived from the element type.

  5. MaxUpdatePhi BFS misses select form (Phase 2.4,
  maxcut_cpp_pass.cpp:476-504)

  When the max-update if-then body contains no exception-throwing calls (e.g.
  the common case if (cut > best) best = cut; with no vector copy), simplifycfg
  eliminates the empty branch and folds it to:
  %best.new = select i1 %cmp, i32 %cut, i32 %best
  The outer header phi then gets %best.new as its latch value — not CutLCSSA or
  MaxPhi directly. The BFS checks PN.incoming_values() for exact pointer
  equality, so %best.new (a SelectInst) never matches. Phase 2.4 returns
  std::nullopt. Demonstrated by tp_val_only.cpp.

  ---
  Minor

  6. Outer loop condition requires ICMP_EQ only (Step 2.2,
  maxcut_cpp_pass.cpp:432)
  if (!OuterICmp || OuterICmp->getPredicate() != ICmpInst::ICMP_EQ)
  The inner loop correctly accepts both EQ and NE (with successor direction
  check), but the outer loop rejects NE unconditionally. If the compiler
  generates icmp ne + swapped successors for the outer loop, you get a false
  negative. Low priority since the optimization pipeline seems to consistently
  emit icmp eq here, but the inconsistency is a fragility.

  7. std::get<> index not verified (Step 1.5, maxcut_cpp_pass.cpp:234-238)
  if (!demangleContains(GetU, "std::get<")) return std::nullopt;
  if (!demangleContains(GetV, "std::get<")) return std::nullopt;
  "std::get<" is a substring match — it matches get<0>, get<1>, get<2>, etc. Two
  calls to get<0> (searching for the first element of the pair twice) would
  pass. Should check that one is get<0, ...> and the other is get<1, ...>.

  8. errs() debug prints in every demangle call (maxcut_cpp_pass.cpp:78)
  errs() << "Demangled: " << D << "\n";
  Called for every function call encountered during scanning. On any real
  codebase this floods stderr. Remove or put behind a debug flag (LLVM_DEBUG).

  9. Phase 2.4 scans inner loop blocks for MaxCompare (maxcut_cpp_pass.cpp:455)
  The loop iterates OuterL->blocks() which includes the inner loop's blocks.
  Scanning inner blocks for the icmp sgt is harmless in practice but could
  theoretically match a comparison inside the inner loop if it accidentally
  satisfies (AccPhi, MaxPhi). Should exclude sub-loop blocks as in Step 1.3.

✻ Churned for 13m 33s

※ recap: Tests are written in test/maxcut/ covering 7 cases (2 true positives, 2 true negatives, 2 confirmed false positives, 2 confirmed false negatives), all verified by running gen_ir.sh. Next: fix the Step 1.9 non-blocking XOR check, the most critical false positive bug. (disable recaps in /config)

❯ /rename loopanalysis 
  ⎿  Session renamed to: loopanalysis
