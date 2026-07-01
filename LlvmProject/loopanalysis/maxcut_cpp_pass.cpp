#include <array>
#include <optional>
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Data structures
struct ScoringLoopMatch {
  Loop* L;
  PHINode* PtrPhi;         // pointer-walk induction var (edge iterator)
  PHINode* AccPhi;         // i32 cut accumulator, preheader init = 0
  Value* EdgesEnd;         // loop-invariant end-of-range pointer
  CallBase* FindU;         // std::find call for endpoint u  (get<0>)
  CallBase* FindV;         // std::find call for endpoint v  (get<1>)
  Value* SubsetAlloca;     // alloca/arg both finds search in
  BinaryOperator* CutAdd;  // add nsw i32 AccPhi, 1
};

struct MaxCutMatch {
  ScoringLoopMatch Inner;

  Loop* OuterL;
  PHINode* OuterPtrPhi;   // pointer-walk iv over subsets
  PHINode* MaxPhi;        // i32 max accumulator, preheader init = 0
  PHINode* CutLCSSA;      // AccPhi LCSSA phi at inner exit
  ICmpInst* MaxCompare;   // icmp sgt CutLCSSA, MaxPhi
  PHINode* MaxUpdatePhi;  // phi that selects max(CutLCSSA, MaxPhi)
  // PHINode* MaxLCSSA;      // MaxPhi LCSSA phi at outer exit (final value)

  // Subset enumeration - future removal target
  Value* SubsetsAlloca;
  Loop* EnumOuterLoop;  // loop that builds SubsetsAlloca
  Loop* EnumInnerLoop;  // nested subset-builder loop

  // Inputs
  Value* EdgesContainer;  // alloca or Argument for the edges vector
};

// Helpers: structural only, no variable-name matching

// Trace through call chains and bitcasts to reach an AllocaInst or Argument.
// Models: result-of-begin(container) → container, etc.
static Value* stripToContainerSource(Value* V) {
  while (V) {
    if (isa<AllocaInst>(V) || isa<Argument>(V))
      return V;
    if (auto* CB = dyn_cast<CallBase>(V)) {
      if (CB->arg_size() == 0)
        return nullptr;
      V = CB->getArgOperand(0);
      continue;
    }
    if (auto* BC = dyn_cast<BitCastInst>(V)) {
      V = BC->getOperand(0);
      continue;
    }
    return nullptr;
  }
  return nullptr;
}

// Mangled-name prefix/substring checks.  These match on the STL function's
// type signature encoded in the ABI name - NOT on user variable names.
static bool demangleContains(const CallBase* CB, StringRef Sub) {
  const Function* F = CB->getCalledFunction();
  std::string demangled = demangle(F->getName());
  StringRef D(demangled);
  errs() << "Demangled: " << D << "\n";
  return D.contains(Sub);
}

// Return the unique non-EH exit block of L, or nullptr.
// getExitBlock() counts landingpad targets as exits; we want only the normal
// one.
static BasicBlock* getNormalExitBlock(Loop* L) {
  BasicBlock* ExitBB = nullptr;
  for (BasicBlock* BB : L->blocks()) {
    for (BasicBlock* Succ : successors(BB)) {
      if (L->contains(Succ) || Succ->isEHPad())
        continue;
      if (ExitBB && ExitBB != Succ)
        return nullptr;  // more than one
      ExitBB = Succ;
    }
  }
  return ExitBB;
}

// Extract the condition of a conditional branch at the end of BB, or nullptr.
static Value* getCondBrCondition(BasicBlock* BB) {
  if (auto* CBR = dyn_cast<CondBrInst>(BB->getTerminator()))
    return CBR->getCondition();
  return nullptr;
}

// Collect every loop in the nest rooted at Root into Out (preorder).
static void collectAllLoops(Loop* Root, SmallVectorImpl<Loop*>& Out) {
  Out.push_back(Root);
  for (Loop* Sub : Root->getSubLoops())
    collectAllLoops(Sub, Out);
}

// Phase 1: match the inner scoring loop
static std::optional<ScoringLoopMatch> matchScoringLoop(Loop* L) {
  BasicBlock* Header = L->getHeader();
  BasicBlock* Preheader = L->getLoopPreheader();
  BasicBlock* Latch = L->getLoopLatch();
  if (!Preheader || !Latch) {
    errs() << "No preheader || latch" << "\n";
    return std::nullopt;
  }

  // 1.1: Exactly 2 header phis: one ptr, one i32 initialised to 0.
  PHINode *PtrPhi = nullptr, *AccPhi = nullptr;
  unsigned PhiCount = 0;
  errs() << Preheader->getName() << ", " << Header->getName() << "\n";
  for (PHINode& PN : Header->phis()) {
    if (++PhiCount > 2)
      return std::nullopt;

    if (PN.getType()->isPointerTy()) {
      if (PtrPhi)
        return std::nullopt;
      PtrPhi = &PN;
    } else if (PN.getType()->isIntegerTy(32)) {
      if (AccPhi)
        return std::nullopt;
      auto* Init =
          dyn_cast<ConstantInt>(PN.getIncomingValueForBlock(Preheader));
      if (!Init || !Init->isZero())
        return std::nullopt;
      AccPhi = &PN;
    } else {
      return std::nullopt;
    }
  }
  if (!PtrPhi || !AccPhi) {
    errs() << "No 2 phis\n";
    return std::nullopt;
  }
  errs() << "Accepted!\n";

  // 1.2: Loop condition
  //  br i1 %cmp.i80.not, label %for.end, label %for.body11
  // We know this comparison is to check subset_begin != end
  auto* ICmp = dyn_cast_or_null<ICmpInst>(getCondBrCondition(Header));
  if (!ICmp || !(ICmp->getPredicate() == ICmpInst::ICMP_EQ ||
                 ICmp->getPredicate() == ICmpInst::ICMP_NE))
    return std::nullopt;
  CondBrInst* BI = dyn_cast<CondBrInst>(Header->getTerminator());
  BasicBlock* TrueSucc = BI->getSuccessor(0);
  BasicBlock* FalseSucc = BI->getSuccessor(1);

  errs() << "Predicate: " << ICmp->getPredicate() << "\n";
  errs() << "FalseSucc: " << FalseSucc->getName()
         << "\tTrueSucc: " << TrueSucc->getName() << "\n";
  errs() << L->contains(FalseSucc) << "\t" << L->contains(TrueSucc) << "\n";

  Value* EdgesEnd = nullptr;
  if (!((ICmp->getPredicate() == ICmpInst::ICMP_NE && L->contains(TrueSucc)) ||
        (ICmp->getPredicate() == ICmpInst::ICMP_EQ &&
         L->contains(FalseSucc)))) {
    return std::nullopt;
  }
  if (ICmp->getOperand(0) == PtrPhi)
    EdgesEnd = ICmp->getOperand(1);
  else if (ICmp->getOperand(1) == PtrPhi)
    EdgesEnd = ICmp->getOperand(0);
  else
    return std::nullopt;
  if (!L->isLoopInvariant(EdgesEnd))
    return std::nullopt;
  errs() << "Phi Init\n";

  // Build a set of blocks that belong to subloops so we don't scan them.
  SmallPtrSet<BasicBlock*, 16> SubLoopBlocks;
  for (Loop* Sub : L->getSubLoops())
    for (BasicBlock* BB : Sub->blocks())
      SubLoopBlocks.insert(BB);

  // 1.3: Exactly 2 std::find calls in the loop's own (non-subloop) blocks.
  // TODO: For now we only consider exact std::find pattern match
  SmallVector<CallBase*, 2> Finds;
  for (BasicBlock* BB : L->blocks()) {
    if (Finds.size() > 2)
      return std::nullopt;
    if (BB == Header || SubLoopBlocks.count(BB))
      continue;
    for (Instruction& I : *BB) {
      if (auto* CB = dyn_cast<CallBase>(&I)) {
        if (Function* Callee = CB->getCalledFunction()) {
          StringRef mangled = Callee->getName();
          if (mangled.find("find") != std::string::npos &&
              demangleContains(CB, "std::find<")) {
            Finds.push_back(CB);
          }
        }
      }
    }
  }
  if (Finds.size() != 2) {
    errs() << "Not enough finds\n";
    return std::nullopt;
  }
  CallBase *FindU = Finds[0], *FindV = Finds[1];
  if (FindU->arg_size() < 3 || FindV->arg_size() < 3)
    return std::nullopt;
  errs() << "All fine(d)\n";

  // 1.4: Both finds search the same container.
  // Handles CSE'd and non-CSE'd begin() by tracing through calls to the source.
  Value* ContU = stripToContainerSource(FindU->getArgOperand(0));
  Value* ContV = stripToContainerSource(FindV->getArgOperand(0));
  if (!ContU || (ContU != ContV))
    return std::nullopt;
  Value* SubsetAlloca = ContU;

  // 1.5: Search values are std::get<0> and std::get<1> of the same pair
  // alloca.
  auto* GetU = dyn_cast<CallBase>(FindU->getArgOperand(2));
  auto* GetV = dyn_cast<CallBase>(FindV->getArgOperand(2));
  if (!GetU || !GetV)
    return std::nullopt;
  if (!demangleContains(GetU, "std::get<"))
    return std::nullopt;
  if (!demangleContains(GetV, "std::get<"))
    return std::nullopt;
  if (GetU->arg_size() < 1 || GetV->arg_size() < 1)
    return std::nullopt;
  if (GetU->getArgOperand(0) != GetV->getArgOperand(0))
    return std::nullopt;

  // 1.6: Conditional add nsw i32 AccPhi, 1 that flows back into AccPhi.
  auto isConstOne = [](Value* V) {
    auto* C = dyn_cast<ConstantInt>(V);
    return C && C->isOne();
  };
  BinaryOperator* CutAdd = nullptr;
  for (BasicBlock* BB : L->blocks()) {
    if (BB == Header || SubLoopBlocks.count(BB))
      continue;
    for (Instruction& I : *BB) {
      auto* BinOp = dyn_cast<BinaryOperator>(&I);
      if (!BinOp || BinOp->getOpcode() != Instruction::Add)
        continue;
      Value *Op0 = BinOp->getOperand(0), *Op1 = BinOp->getOperand(1);
      if (!((Op0 == AccPhi && isConstOne(Op1)) ||
            (Op1 == AccPhi && isConstOne(Op0))))
        continue;
      // Verify the add reaches AccPhi's backedge, possibly via a merge phi.
      Value* Back = AccPhi->getIncomingValueForBlock(Latch);
      bool Feeds = (Back == BinOp);
      if (!Feeds)
        if (auto* MP = dyn_cast<PHINode>(Back))
          for (Value* V : MP->incoming_values())
            if (V == BinOp) {
              Feeds = true;
              break;
            }
      if (Feeds) {
        CutAdd = BinOp;
        break;
      }
    }
  }
  if (!CutAdd)
    return std::nullopt;

  // 1.8: CutAdd's block must have exactly 2 predecessors within the loop.
  //        In the correct XOR diamond (||), two paths merge here:
  //          u_in=T, v_in=F  →  cut block
  //          u_in=F, v_in=T  →  cut block
  //        With && (always-false), only one path can reach this block, so
  //        pred_count == 1 and we correctly reject.
  //  OR
  // TODO: reduced to XOR by simplifycfg and instcombine => 1 predecessor
  SmallVector<BasicBlock*, 2> PredsCutAdd;
  for (BasicBlock* Pred : predecessors(CutAdd->getParent())) {
    if (L->contains(Pred))
      PredsCutAdd.push_back(Pred);
  }
  if (PredsCutAdd.size() > 2)
    return std::nullopt;

  // 1.9: XOR gate conditions: u_in_subset and v_in_subset.
  for (BasicBlock* Pred : PredsCutAdd) {
    auto* CBR = dyn_cast<CondBrInst>(Pred->getTerminator());
    Value* BC = CBR->getCondition();
    BasicBlock* TrueLabel = dyn_cast<BasicBlock>(CBR->getOperand(1));
    BasicBlock* FalseLabel = dyn_cast<BasicBlock>(CBR->getOperand(2));

    errs() << *BC << "\n"
           << (TrueLabel == CutAdd->getParent()) << "\n"
           << (FalseLabel == Latch) << "\n";

    Instruction* XorInst = dyn_cast_or_null<Instruction>(BC);
    if (!XorInst) {
      errs() << "Not an inst!?\n";
      continue;
    }
    if (XorInst->getOpcode() != Instruction::Xor) {
      continue;
    }

    errs() << "\nInst: " << XorInst->getOpcodeName() << "\n"
           << *XorInst->getOperand(0) << "\n"
           << *XorInst->getOperand(1) << "\n";

    std::array<bool, 2> validFinds{false, false};
    auto isValidCmp = [&](Value* V) {
      auto* C = dyn_cast_or_null<ICmpInst>(V);
      if (!C || C->getPredicate() != ICmpInst::ICMP_NE)
        return false;
      Value* VecEnd = nullptr;
      int idx = 0;

      // TODO: Is this valid comparison?
      if (FindU == C->getOperand(0) || FindU == C->getOperand(1)) {
        if (validFinds[idx])
          return false;
        VecEnd =
            FindU == C->getOperand(0) ? C->getOperand(1) : C->getOperand(0);
      } else if (FindV == C->getOperand(0) || FindV == C->getOperand(1)) {
        if (validFinds[idx = 1])
          return false;
        VecEnd =
            FindV == C->getOperand(0) ? C->getOperand(1) : C->getOperand(0);
      } else
        return false;

      if (auto* CB = dyn_cast<CallBase>(VecEnd)) {
        if (Function* Callee = CB->getCalledFunction()) {
          StringRef mangled = Callee->getName();
          if (mangled.find("end") != std::string::npos &&
              demangleContains(
                  CB, "std::vector<int, std::allocator<int>>::end()")) {
            validFinds[idx] = true;
          }
        }
      }

      return true;
    };

    if (!(isValidCmp(XorInst->getOperand(0)) &&
          isValidCmp(XorInst->getOperand(1))))
      return std::nullopt;

    // Now that we know its an xor of vec::find != vec::end values
    // We can say true condition shall goto inc and false to latch
    if (TrueLabel != CutAdd->getParent() && FalseLabel != Latch)
      return std::nullopt;
  }

  // 1.7: Latch GEP: getelementptr <StructType>, PtrPhi, i32 1.
  //        The pair<int,int> element type distinguishes this from outer loops.
  GetElementPtrInst* LatchGEP = nullptr;
  for (Instruction& I : *Latch) {
    auto* GEP = dyn_cast<GetElementPtrInst>(&I);
    if (!GEP || GEP->getPointerOperand() != PtrPhi)
      continue;
    if (GEP->getNumIndices() != 1)
      continue;
    auto* Idx = dyn_cast<ConstantInt>(GEP->idx_begin()->get());
    if (!Idx || !Idx->equalsInt(8))
      continue;
    if (!GEP->getSourceElementType()->isIntegerTy(8))
      continue;
    if (PtrPhi->getIncomingValueForBlock(Latch) != GEP)
      continue;
    LatchGEP = GEP;
    break;
  }
  if (!LatchGEP)
    return std::nullopt;

  return ScoringLoopMatch{L,     PtrPhi, AccPhi,       EdgesEnd,
                          FindU, FindV,  SubsetAlloca, CutAdd};
}

// Phase 2: match the enclosing max-tracking loop
static std::optional<MaxCutMatch> matchMaxCut(const ScoringLoopMatch& Inner,
                                              LoopInfo& LI) {
  Loop* OuterL = Inner.L->getParentLoop();
  if (!OuterL)
    return std::nullopt;

  BasicBlock* OuterHeader = OuterL->getHeader();
  BasicBlock* OuterPreheader = OuterL->getLoopPreheader();
  BasicBlock* OuterLatch = OuterL->getLoopLatch();
  if (!OuterPreheader || !OuterLatch)
    return std::nullopt;

  // 2.1: Outer header
  // exactly 2 phis: ptr + i32(0).
  PHINode *OuterPtrPhi = nullptr, *MaxPhi = nullptr;
  unsigned PhiCount = 0;
  for (PHINode& PN : OuterHeader->phis()) {
    if (++PhiCount > 2)
      return std::nullopt;
    if (PN.getType()->isPointerTy()) {
      if (OuterPtrPhi)
        return std::nullopt;
      OuterPtrPhi = &PN;
    } else if (PN.getType()->isIntegerTy(32)) {
      auto* Init =
          dyn_cast<ConstantInt>(PN.getIncomingValueForBlock(OuterPreheader));
      if (!Init || !Init->isZero())
        return std::nullopt;
      if (MaxPhi)
        return std::nullopt;
      MaxPhi = &PN;
    } else {
      return std::nullopt;
    }
  }
  if (!OuterPtrPhi || !MaxPhi)
    return std::nullopt;

  // 2.2: Outer condition
  // icmp eq ptr OuterPtrPhi, <loop-invariant end>
  auto* OuterICmp = dyn_cast_or_null<ICmpInst>(getCondBrCondition(OuterHeader));
  if (!OuterICmp || OuterICmp->getPredicate() != ICmpInst::ICMP_EQ)
    return std::nullopt;
  Value* SubsetsEnd = nullptr;
  if (OuterICmp->getOperand(0) == OuterPtrPhi)
    SubsetsEnd = OuterICmp->getOperand(1);
  else if (OuterICmp->getOperand(1) == OuterPtrPhi)
    SubsetsEnd = OuterICmp->getOperand(0);
  else
    return std::nullopt;
  if (!OuterL->isLoopInvariant(SubsetsEnd))
    return std::nullopt;
  errs() << "2.2 done\n";

  // 2.3: LCSSA phi for the cut accumulator at the inner loop's normal exit.
  BasicBlock* InnerExit = getNormalExitBlock(Inner.L);
  if (!InnerExit)
    return std::nullopt;
  PHINode* CutLCSSA = Inner.AccPhi;
  errs() << "2.3 done\n";

  // 2.4: icmp sgt(CutLCSSA, MaxPhi) and a merge phi selecting the max value.
  ICmpInst* MaxCompare = nullptr;
  for (BasicBlock* BB : OuterL->blocks()) {
    if (BB == OuterHeader || BB == OuterLatch)
      continue;
    for (Instruction& I : *BB) {
      auto* Cmp = dyn_cast<ICmpInst>(&I);
      if (!Cmp || Cmp->getPredicate() != ICmpInst::ICMP_SGT)
        continue;
      Value *A = Cmp->getOperand(0), *B = Cmp->getOperand(1);
      if ((A == CutLCSSA && B == MaxPhi) || (A == MaxPhi && B == CutLCSSA)) {
        MaxCompare = Cmp;
        break;
      }
    }
    if (MaxCompare)
      break;
  }
  if (!MaxCompare)
    return std::nullopt;
  errs() << "SGT Compared\n";

  // BFS from MaxCompare's successors to find the merge phi
  // (phi with one incoming = CutLCSSA, one = MaxPhi).
  PHINode* MaxUpdatePhi = nullptr;
  {
    SmallVector<BasicBlock*, 8> Worklist(successors(MaxCompare->getParent()));
    SmallPtrSet<BasicBlock*, 8> Seen;
    while (!Worklist.empty() && !MaxUpdatePhi) {
      BasicBlock* BB = Worklist.pop_back_val();
      if (!Seen.insert(BB).second || !OuterL->contains(BB))
        continue;
      for (PHINode& PN : BB->phis()) {
        bool HasCut = false, HasMax = false;
        for (Value* V : PN.incoming_values()) {
          if (V == CutLCSSA)
            HasCut = true;
          if (V == MaxPhi)
            HasMax = true;
        }
        if (HasCut && HasMax) {
          MaxUpdatePhi = &PN;
          break;
        }
      }
      if (!MaxUpdatePhi)
        for (BasicBlock* S : successors(BB))
          Worklist.push_back(S);
    }
  }
  if (!MaxUpdatePhi)
    return std::nullopt;
  errs() << "BFS done\n";

  // 2.5: Outer latch GEP advances OuterPtrPhi by 1 struct element.
  //        Element type is the vector struct (larger than a pair).
  GetElementPtrInst* OuterLatchGEP = nullptr;
  for (Instruction& I : *OuterLatch) {
    auto* GEP = dyn_cast<GetElementPtrInst>(&I);
    if (!GEP || GEP->getPointerOperand() != OuterPtrPhi)
      continue;
    if (GEP->getNumIndices() != 1)
      continue;
    auto* Idx = dyn_cast<ConstantInt>(GEP->idx_begin()->get());
    if (!Idx || !Idx->equalsInt(24))
      continue;
    if (!GEP->getSourceElementType()->isIntegerTy(8))
      continue;
    if (OuterPtrPhi->getIncomingValueForBlock(OuterLatch) != GEP)
      continue;
    OuterLatchGEP = GEP;
    break;
  }
  if (!OuterLatchGEP)
    return std::nullopt;
  errs() << "GEP Done\n";

  // 2.6: Identify the subset enumeration loops (future removal targets).
  Value* SubsetsAlloca = stripToContainerSource(
      OuterPtrPhi->getIncomingValueForBlock(OuterPreheader));

  Loop *EnumOuter = nullptr, *EnumInner = nullptr;
  if (SubsetsAlloca) {
    auto hasPushBackTo = [&](Loop* Lp, Value* Target) {
      for (BasicBlock* BB : Lp->blocks())
        for (Instruction& I : *BB)
          if (auto* CB = dyn_cast<CallBase>(&I))
            if (CB->arg_size() >= 1 && CB->getArgOperand(0) == Target &&
                demangleContains(CB, "push_back"))
              return true;
      return false;
    };

    for (Loop* TopL : LI) {
      if (TopL == OuterL)
        continue;
      errs() << *TopL << "\n";
      if (!hasPushBackTo(TopL, SubsetsAlloca))
        continue;
      EnumOuter = TopL;
      for (Loop* Sub : TopL->getSubLoops())
        if (hasPushBackTo(Sub, SubsetsAlloca)) {
          EnumInner = Sub;
          break;
        }
      break;
    }
  }

  // 2.7: Identify the edges container from the inner loop's preheader.
  Value* EdgesContainer = nullptr;
  if (BasicBlock* InnerPreheader = Inner.L->getLoopPreheader()) {
    Value* BeginCall = Inner.PtrPhi->getIncomingValueForBlock(InnerPreheader);
    EdgesContainer = stripToContainerSource(BeginCall);
  }

  return MaxCutMatch{Inner,     OuterL,     OuterPtrPhi,   MaxPhi,
                     CutLCSSA,  MaxCompare, MaxUpdatePhi,  SubsetsAlloca,
                     EnumOuter, EnumInner,  EdgesContainer};
}

// Reporting
static void printMatch(const MaxCutMatch& M) {
  errs() << "\n  *** MaxCut-CPP pattern matched ***\n";

  errs() << "  -- Scoring loop --\n";
  errs() << "    header      : " << M.Inner.L->getHeader()->getName() << "\n";
  errs() << "    edge iter   : " << *M.Inner.PtrPhi << "\n";
  errs() << "    edges end   : " << *M.Inner.EdgesEnd << "\n";
  errs() << "    accumulator : " << *M.Inner.AccPhi << "\n";
  errs() << "    find(u)     : " << *M.Inner.FindU << "\n";
  errs() << "    find(v)     : " << *M.Inner.FindV << "\n";
  errs() << "    subset cont : " << *M.Inner.SubsetAlloca << "\n";
  errs() << "    cut +1      : " << *M.Inner.CutAdd << "\n";

  errs() << "  -- Outer loop --\n";
  errs() << "    header      : " << M.OuterL->getHeader()->getName() << "\n";
  errs() << "    subset iter : " << *M.OuterPtrPhi << "\n";
  errs() << "    max accum   : " << *M.MaxPhi << "\n";
  errs() << "    cut lcssa   : " << *M.CutLCSSA << "\n";
  errs() << "    max cmp     : " << *M.MaxCompare << "\n";
  errs() << "    max update  : " << *M.MaxUpdatePhi << "\n";

  errs() << "  -- Subset enumeration (future removal target) --\n";
  if (M.SubsetsAlloca) {
    errs() << "    subsets     : " << *M.SubsetsAlloca << "\n";
    errs() << "    enum outer  : "
           << (M.EnumOuterLoop ? M.EnumOuterLoop->getHeader()->getName()
                               : StringRef("not found"))
           << "\n";
    errs() << "    enum inner  : "
           << (M.EnumInnerLoop ? M.EnumInnerLoop->getHeader()->getName()
                               : StringRef("not found"))
           << "\n";
  } else {
    errs() << "    (could not trace subsets alloca)\n";
  }

  errs() << "  -- Inputs --\n";
  if (M.EdgesContainer)
    errs() << "    edges src   : " << *M.EdgesContainer << "\n";
  else
    errs() << "    (could not trace edges container)\n";
  errs() << "\n";
}

// Pass
namespace {
struct MaxCutCppPass : PassInfoMixin<MaxCutCppPass> {
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    LoopInfo& LI = AM.getResult<LoopAnalysis>(F);
    errs() << "[MaxCut-CPP] scanning: " << F.getName() << "\n";

    SmallVector<MaxCutMatch, 2> Matches;

    // Collect all loops across the entire nest, try each as a scoring loop.
    SmallVector<Loop*, 16> AllLoops;
    for (Loop* TopL : LI)
      collectAllLoops(TopL, AllLoops);

    errs() << AllLoops.size() << "\n";
    for (Loop* L : AllLoops) {
      auto Scoring = matchScoringLoop(L);
      if (!Scoring)
        continue;

      auto Full = matchMaxCut(*Scoring, LI);
      if (!Full) {
        errs() << "  [note] scoring loop in " << L->getHeader()->getName()
               << " has no enclosing max-tracking loop\n";
        continue;
      }

      // Deduplicate: one report per outer loop.
      bool Dup = false;
      for (auto& E : Matches)
        if (E.OuterL == Full->OuterL) {
          Dup = true;
          break;
        }
      if (!Dup)
        Matches.push_back(*Full);
    }

    if (Matches.empty())
      errs() << "  no MaxCut-CPP pattern found.\n";
    else
      for (auto& M : Matches)
        printMatch(M);

    return PreservedAnalyses::all();
  }
};
}  // namespace

// Exported so pass.cpp can call this from the shared llvmGetPassPluginInfo.
void registerMaxCutCppPass(PassBuilder& PB) {
  PB.registerPipelineParsingCallback(
      [](StringRef Name, FunctionPassManager& FPM,
         ArrayRef<PassBuilder::PipelineElement>) -> bool {
        if (Name == "maxcut-cpp-pass") {
          FPM.addPass(MaxCutCppPass());
          return true;
        }
        return false;
      });
}
