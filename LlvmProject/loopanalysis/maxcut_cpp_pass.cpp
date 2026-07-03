#include <array>
#include <cstdint>
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
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "maxcut-cpp"  // -debug-only=maxcut-cpp

using namespace llvm;

// Data structures
struct ScoringLoopMatch {
  Loop* L;
  PHINode* PtrPhi;               // pointer-walk induction var (edge iterator)
  PHINode* AccPhi;               // i32 cut accumulator, preheader init = 0
  Value* EdgesEnd;               // loop-invariant end-of-range pointer
  CallBase* FindU;               // std::find call for endpoint u  (get<0>)
  CallBase* FindV;               // std::find call for endpoint v  (get<1>)
  Value* SubsetAlloca;           // alloca/arg both finds search in
  BinaryOperator* CutAdd;        // add i32 AccPhi, (1 or zext(xor))
  APInt Increment{64, 1, true};  // cut increment weight; 1 by default
};

struct MaxCutMatch {
  ScoringLoopMatch Inner;

  Loop* OuterL;
  PHINode* OuterPtrPhi;  // pointer-walk iv over subsets
  PHINode* MaxPhi;       // i32 max accumulator, preheader init = 0
  PHINode*
      CutLCSSA;  // AccPhi LCSSA phi at inner exit (or Inner.AccPhi directly)
  ICmpInst*
      MaxCompare;  // icmp sgt CutLCSSA, MaxPhi — null when smax intrinsic used
  Value* MaxUpdatePhi;  // phi/select/smax that produces max(CutLCSSA, MaxPhi)

  // Subset enumeration - future removal target
  Value* SubsetsAlloca;
  Loop* EnumOuterLoop;
  Loop* EnumInnerLoop;

  // Inputs
  Value* EdgesContainer;
};

// ---- Helpers ---------------------------------------------------------------

// Trace through call chains and bitcasts to reach an AllocaInst or Argument.
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

// When find() searches for a cast value (e.g. `(int)(long long)`), the search
// arg is an i32 alloca populated by: store (trunc/cast (load get<N>(...))).
// Trace back through those layers to the get<> CallBase, or return nullptr.
static CallBase* traceGetThroughCast(Value* Arg) {
  auto* Alloca = dyn_cast<AllocaInst>(Arg);
  if (!Alloca)
    return nullptr;
  for (User* U : Alloca->users()) {
    auto* SI = dyn_cast<StoreInst>(U);
    if (!SI || SI->getPointerOperand() != Alloca)
      continue;
    Value* Stored = SI->getValueOperand();
    while (auto* Cast = dyn_cast<CastInst>(Stored))
      Stored = Cast->getOperand(0);
    auto* LI = dyn_cast<LoadInst>(Stored);
    if (!LI)
      continue;
    if (auto* CB = dyn_cast<CallBase>(LI->getPointerOperand()))
      return CB;
  }
  return nullptr;
}

static bool demangleContains(const CallBase* CB, StringRef Sub) {
  const Function* F = CB->getCalledFunction();
  std::string demangled = demangle(F->getName());
  LLVM_DEBUG(dbgs() << "Demangled: " << demangled << "\n");
  return StringRef(demangled).contains(Sub);
}

// Return the unique non-EH exit block of L, or nullptr.
static BasicBlock* getNormalExitBlock(Loop* L) {
  BasicBlock* ExitBB = nullptr;
  for (BasicBlock* BB : L->blocks()) {
    for (BasicBlock* Succ : successors(BB)) {
      if (L->contains(Succ) || Succ->isEHPad())
        continue;
      if (ExitBB && ExitBB != Succ)
        return nullptr;
      ExitBB = Succ;
    }
  }
  return ExitBB;
}

static Value* getCondBrCondition(BasicBlock* BB) {
  if (auto* CBR = dyn_cast<CondBrInst>(BB->getTerminator()))
    return CBR->getCondition();
  return nullptr;
}

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
    LLVM_DEBUG(dbgs() << "No preheader || latch\n");
    return std::nullopt;
  }

  // 1.1: Exactly 2 header phis: one ptr, one i32 initialised to 0.
  PHINode *PtrPhi = nullptr, *AccPhi = nullptr;
  unsigned PhiCount = 0;
  LLVM_DEBUG(dbgs() << Preheader->getName() << ", " << Header->getName()
                    << "\n");
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
    LLVM_DEBUG(dbgs() << "No 2 phis\n");
    return std::nullopt;
  }
  LLVM_DEBUG(dbgs() << "Accepted!\n");

  // 1.2: Loop condition
  // accept EQ or NE with correct successor direction.
  auto* ICmp = dyn_cast_or_null<ICmpInst>(getCondBrCondition(Header));
  if (!ICmp || !(ICmp->getPredicate() == ICmpInst::ICMP_EQ ||
                 ICmp->getPredicate() == ICmpInst::ICMP_NE))
    return std::nullopt;
  CondBrInst* BI = dyn_cast<CondBrInst>(Header->getTerminator());
  BasicBlock* TrueSucc = BI->getSuccessor(0);
  BasicBlock* FalseSucc = BI->getSuccessor(1);
  LLVM_DEBUG(dbgs() << "Predicate: " << ICmp->getPredicate() << "\n"
                    << "FalseSucc: " << FalseSucc->getName()
                    << "\tTrueSucc: " << TrueSucc->getName() << "\n"
                    << L->contains(FalseSucc) << "\t" << L->contains(TrueSucc)
                    << "\n");
  if (!((ICmp->getPredicate() == ICmpInst::ICMP_NE && L->contains(TrueSucc)) ||
        (ICmp->getPredicate() == ICmpInst::ICMP_EQ && L->contains(FalseSucc))))
    return std::nullopt;

  Value* EdgesEnd = nullptr;
  if (ICmp->getOperand(0) == PtrPhi)
    EdgesEnd = ICmp->getOperand(1);
  else if (ICmp->getOperand(1) == PtrPhi)
    EdgesEnd = ICmp->getOperand(0);
  else
    return std::nullopt;
  if (!L->isLoopInvariant(EdgesEnd))
    return std::nullopt;
  LLVM_DEBUG(dbgs() << "Phi Init\n");

  // Build subloop block set so we can skip them in inner scans.
  SmallPtrSet<BasicBlock*, 16> SubLoopBlocks;
  for (Loop* Sub : L->getSubLoops())
    for (BasicBlock* BB : Sub->blocks())
      SubLoopBlocks.insert(BB);

  // 1.3: Exactly 2 std::find calls in the loop's own (non-subloop) blocks.
  // TODO: Maybe better if 2 finds on subsetAlloca and not in general 2
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
          bool isMembership =
              (mangled.find("find") != std::string::npos &&
               demangleContains(CB, "std::find<")) ||
              (mangled.find("count") != std::string::npos &&
               demangleContains(CB, "std::count<"));
          if (isMembership)
            Finds.push_back(CB);
        }
      }
    }
  }
  if (Finds.size() != 2) {
    LLVM_DEBUG(dbgs() << "Not enough finds\n");
    return std::nullopt;
  }
  CallBase *FindU = Finds[0], *FindV = Finds[1];
  if (FindU->arg_size() < 3 || FindV->arg_size() < 3)
    return std::nullopt;
  LLVM_DEBUG(dbgs() << "All fine(d)\n");

  // 1.4: Both finds search the same container.
  Value* ContU = stripToContainerSource(FindU->getArgOperand(0));
  Value* ContV = stripToContainerSource(FindV->getArgOperand(0));
  if (!ContU || (ContU != ContV))
    return std::nullopt;
  Value* SubsetAlloca = ContU;

  // 1.5: Search values are std::get<0> and std::get<1> of the same pair alloca.
  // Direct: find(..., get<N>(pair))
  // Cast:   find(..., ref_tmp) where ref_tmp <-
  // store(trunc(load(get<N>(pair))))
  auto* GetU = dyn_cast<CallBase>(FindU->getArgOperand(2));
  if (!GetU)
    GetU = traceGetThroughCast(FindU->getArgOperand(2));
  auto* GetV = dyn_cast<CallBase>(FindV->getArgOperand(2));
  if (!GetV)
    GetV = traceGetThroughCast(FindV->getArgOperand(2));
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

  // fix-7: verify one call is get<0> and the other is get<1>.
  auto isGetIndex = [](const CallBase* CB, unsigned N) {
    const Function* F = CB->getCalledFunction();
    if (!F)
      return false;
    std::string D = demangle(F->getName());
    std::string Needle = "std::get<" + std::to_string(N);
    return StringRef(D).contains(Needle);
  };
  if (!((isGetIndex(GetU, 0) && isGetIndex(GetV, 1)) ||
        (isGetIndex(GetU, 1) && isGetIndex(GetV, 0))))
    return std::nullopt;

  // TODO: Hoist isValidCmp so it can be used in both the branching and
  // branchless XOR gate paths below.
  std::array<bool, 2> validFinds{false, false};
  auto isValidCmp = [&](Value* V) {
    auto* C = dyn_cast_or_null<ICmpInst>(V);
    if (!C)
      return false;
    ICmpInst::Predicate Pred = C->getPredicate();
    if (Pred != ICmpInst::ICMP_NE && Pred != ICmpInst::ICMP_SGT)
      return false;
    Value* VecEnd = nullptr;
    int idx = 0;
    if (FindU == C->getOperand(0) || FindU == C->getOperand(1)) {
      if (validFinds[idx])
        return false;
      VecEnd = FindU == C->getOperand(0) ? C->getOperand(1) : C->getOperand(0);
    } else if (FindV == C->getOperand(0) || FindV == C->getOperand(1)) {
      if (validFinds[idx = 1])
        return false;
      VecEnd = FindV == C->getOperand(0) ? C->getOperand(1) : C->getOperand(0);
    } else
      return false;
    // find() != end()
    if (Pred == ICmpInst::ICMP_NE) {
      if (auto* CB = dyn_cast<CallBase>(VecEnd)) {
        if (Function* Callee = CB->getCalledFunction()) {
          StringRef mangled = Callee->getName();
          validFinds[idx] = mangled.find("end") != std::string::npos &&
                            demangleContains(CB, ">>::end()");
        }
      }
    }
    // count() != 0 or count() > 0
    if (!validFinds[idx]) {
      if (auto* Z = dyn_cast<ConstantInt>(VecEnd))
        validFinds[idx] = Z->isZero();
    }
    return validFinds[idx];
  };

  // 1.6: CutAdd: add i32 AccPhi, <increment> that flows back into AccPhi.
  // Branching form : increment = ConstantInt(1)
  // Branchless form: increment = zext i1 <XOR result>  (fix-5b: LLVM folds
  //   `if (u_in ^ v_in) cut++` to `cut += zext(u_in ^ v_in)` when the
  //   if-body has no exception-throwing calls)
  APInt IncWeight{64, 1, true};
  auto isIncrement = [&](Value* V) -> bool {
    if (auto* Z = dyn_cast<ZExtInst>(V))
      return Z->getSrcTy()->isIntegerTy(1) && Z->getType()->isIntegerTy(32);
    if (auto* C = dyn_cast<ConstantInt>(V); C) {
      IncWeight = C->getValue();
      LLVM_DEBUG(dbgs() << "IncWeight Found: " << IncWeight << "\n");
      return true;
    }
    return false;
  };

  BinaryOperator* CutAdd = nullptr;
  Value* IncrVal = nullptr;
  for (BasicBlock* BB : L->blocks()) {
    if (BB == Header || SubLoopBlocks.count(BB))
      continue;
    for (Instruction& I : *BB) {
      auto* BinOp = dyn_cast<BinaryOperator>(&I);
      if (!BinOp || BinOp->getOpcode() != Instruction::Add)
        continue;
      Value *Op0 = BinOp->getOperand(0), *Op1 = BinOp->getOperand(1);
      Value* Incr = nullptr;
      if (Op0 == AccPhi && isIncrement(Op1))
        Incr = Op1;
      else if (Op1 == AccPhi && isIncrement(Op0))
        Incr = Op0;
      else
        continue;
      LLVM_DEBUG(dbgs() << "BinOp: " << *BinOp << "\n");
      Value* Back = AccPhi->getIncomingValueForBlock(Latch);
      LLVM_DEBUG(dbgs() << "Back: " << *Back << "\n");
      bool Feeds = (Back == BinOp);
      if (!Feeds) {
        if (auto* MP = dyn_cast<PHINode>(Back)) {
          for (Value* V : MP->incoming_values()) {
            LLVM_DEBUG(dbgs() << "V: " << *V << "\n");
            if (V == BinOp) {
              Feeds = true;
              break;
            }
          }
        } else if (auto* MP = dyn_cast<SelectInst>(Back)) {
          for (Value* V : MP->operand_values()) {
            LLVM_DEBUG(dbgs() << "V: " << *V << "\n");
            if (V == BinOp) {
              Feeds = true;
              break;
            }
          }
        }
      }
      if (Feeds) {
        CutAdd = BinOp;
        IncrVal = Incr;
        break;
      }
    }
    if (CutAdd)
      break;
  }
  if (!CutAdd)
    return std::nullopt;
  LLVM_DEBUG(dbgs() << "IncrVal: " << *IncrVal << "\n");
  LLVM_DEBUG(dbgs() << "CutAdd: " << *CutAdd << "\n");

  // 1.9: XOR gate: verify exactly-one-endpoint semantics.
  //
  // Branchless form: IncrVal is `zext i1 (xor i1 ...)`; validate the XOR
  //   instruction directly as it is an operand of the add.
  //
  // Branchless weighted form: IncrVal is a Constant other than 1, if branch is
  //   coallesced to a select instruction
  //
  // Branching form: IncrVal is ConstantInt(1); locate the CondBrInst
  //   predecessor of CutAdd's block and validate its condition.
  bool validXorOp = false;

  if (auto* ZE = dyn_cast<ZExtInst>(IncrVal)) {
    auto* XorInst = dyn_cast<BinaryOperator>(ZE->getOperand(0));
    validXorOp = (XorInst && XorInst->getOpcode() == Instruction::Xor &&
                  isValidCmp(XorInst->getOperand(0)) &&
                  isValidCmp(XorInst->getOperand(1)));
  } else if (!IncWeight.isOne()) {
    // This was validated by the select instruction 1.6
    validXorOp = true;
  } else {
    SmallVector<BasicBlock*, 2> PredsCutAdd;
    for (BasicBlock* Pred : predecessors(CutAdd->getParent()))
      if (L->contains(Pred))
        PredsCutAdd.push_back(Pred);
    if (PredsCutAdd.size() > 2)
      return std::nullopt;

    for (BasicBlock* Pred : PredsCutAdd) {
      auto* CBR = dyn_cast<CondBrInst>(Pred->getTerminator());
      if (!CBR)
        continue;
      Value* BC = CBR->getCondition();
      BasicBlock* TrueLabel = CBR->getSuccessor(0);
      BasicBlock* FalseLabel = CBR->getSuccessor(1);
      LLVM_DEBUG(dbgs() << *BC << "\n"
                        << (TrueLabel == CutAdd->getParent()) << "\n"
                        << (FalseLabel == Latch) << "\n");

      auto* XorInst = dyn_cast_or_null<BinaryOperator>(BC);
      if (!XorInst || XorInst->getOpcode() != Instruction::Xor) {
        LLVM_DEBUG(dbgs() << "Not XOR!\n");
        continue;
      }
      LLVM_DEBUG(dbgs() << "\nInst: " << XorInst->getOpcodeName() << "\n"
                        << *XorInst->getOperand(0) << "\n"
                        << *XorInst->getOperand(1) << "\n");
      if (!(isValidCmp(XorInst->getOperand(0)) &&
            isValidCmp(XorInst->getOperand(1))))
        return std::nullopt;
      if (TrueLabel != CutAdd->getParent() && FalseLabel != Latch)
        return std::nullopt;
      validXorOp = true;
      break;
    }
  }
  if (!validXorOp)
    return std::nullopt;

  // 1.7: Latch GEP: getelementptr i8, PtrPhi, <stride derived from DataLayout>.
  GetElementPtrInst* LatchGEP = nullptr;
  auto* PairAlloca = dyn_cast_or_null<AllocaInst>(
      stripToContainerSource(GetU->getArgOperand(0)));
  if (!PairAlloca)
    return std::nullopt;
  Type* PairTy = PairAlloca->getAllocatedType();
  const DataLayout& DL = Header->getModule()->getDataLayout();
  uint64_t ExpectedStride = DL.getTypeAllocSize(PairTy);

  for (Instruction& I : *Latch) {
    auto* GEP = dyn_cast<GetElementPtrInst>(&I);
    if (!GEP || GEP->getPointerOperand() != PtrPhi)
      continue;
    if (GEP->getNumIndices() != 1)
      continue;
    auto* Idx = dyn_cast<ConstantInt>(GEP->idx_begin()->get());
    if (!Idx || !Idx->equalsInt(ExpectedStride))
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

  return ScoringLoopMatch{L,     PtrPhi,       AccPhi, EdgesEnd, FindU,
                          FindV, SubsetAlloca, CutAdd, IncWeight};
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
      // TODO: generalize to non-zero init (e.g. best = -1 as sentinel);
      // currently only isZero() is accepted, so fn_nonzero_init is a false negative.
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
  // accept EQ and NE with direction check.
  auto* OuterICmp = dyn_cast_or_null<ICmpInst>(getCondBrCondition(OuterHeader));
  if (!OuterICmp || !(OuterICmp->getPredicate() == ICmpInst::ICMP_EQ ||
                      OuterICmp->getPredicate() == ICmpInst::ICMP_NE))
    return std::nullopt;
  CondBrInst* OuterBI = dyn_cast<CondBrInst>(OuterHeader->getTerminator());
  BasicBlock* OuterTrueSucc = OuterBI->getSuccessor(0);
  BasicBlock* OuterFalseSucc = OuterBI->getSuccessor(1);
  if (!((OuterICmp->getPredicate() == ICmpInst::ICMP_NE &&
         OuterL->contains(OuterTrueSucc)) ||
        (OuterICmp->getPredicate() == ICmpInst::ICMP_EQ &&
         OuterL->contains(OuterFalseSucc))))
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
  LLVM_DEBUG(dbgs() << "2.2 done\n");

  // 2.3: Cut accumulator value used after the inner loop exits.
  BasicBlock* InnerExit = getNormalExitBlock(Inner.L);
  if (!InnerExit)
    return std::nullopt;
  PHINode* CutLCSSA = Inner.AccPhi;
  LLVM_DEBUG(dbgs() << "2.3 done\n");

  // fix-9: Build set of all inner-subloop blocks to avoid false matches inside
  // the scoring loop when scanning for the max-selection instruction.
  SmallPtrSet<BasicBlock*, 16> InnerSubLoopBlocks;
  for (Loop* Sub : OuterL->getSubLoops())
    for (BasicBlock* BB : Sub->blocks())
      InnerSubLoopBlocks.insert(BB);

  // 2.4: Find the max-selection instruction. Three forms:
  //   (a) icmp sgt + phi merge (tp_basic: update block with vector assignments)
  //   (b) icmp sgt + select    (fix-5: simplifycfg folds empty update block)
  //   (c) @llvm.smax.i32       (fix-5b: LLVM >=20 may emit smax intrinsic
  //                             instead of icmp+select for value-only tracking)
  //
  // The outer latch is NOT excluded: when form (c) fires, the smax lives in
  // the same block as the backedge GEP (i.e., the latch itself).
  ICmpInst* MaxCompare = nullptr;
  Value* MaxUpdatePhi = nullptr;

  for (BasicBlock* BB : OuterL->blocks()) {
    if (BB == OuterHeader || InnerSubLoopBlocks.count(BB))
      continue;
    for (Instruction& I : *BB) {
      if (!MaxCompare) {
        if (auto* Cmp = dyn_cast<ICmpInst>(&I)) {
          if (Cmp->getPredicate() == ICmpInst::ICMP_SGT) {
            Value *A = Cmp->getOperand(0), *B = Cmp->getOperand(1);
            if ((A == CutLCSSA && B == MaxPhi) ||
                (A == MaxPhi && B == CutLCSSA))
              MaxCompare = Cmp;
          }
        }
      }
      if (!MaxUpdatePhi) {
        if (auto* CB = dyn_cast<CallBase>(&I)) {
          if (Function* Callee = CB->getCalledFunction();
              Callee && CB->arg_size() == 2 &&
              Callee->getName().contains("llvm.smax")) {
            Value *A = CB->getArgOperand(0), *B = CB->getArgOperand(1);
            if ((A == CutLCSSA && B == MaxPhi) ||
                (A == MaxPhi && B == CutLCSSA))
              MaxUpdatePhi = CB;
          }
        }
      }
    }
    if (MaxCompare || MaxUpdatePhi)
      break;
  }

  if (!MaxCompare && !MaxUpdatePhi)
    return std::nullopt;
  LLVM_DEBUG(dbgs() << "SGT or smax found\n");

  // For forms (a)/(b): BFS + SelectInst fallback.
  if (MaxCompare && !MaxUpdatePhi) {
    // BFS from MaxCompare's block searching for a phi that merges CutLCSSA
    // and MaxPhi (form a).
    {
      PHINode* PhiForm = nullptr;
      SmallVector<BasicBlock*, 8> Worklist(successors(MaxCompare->getParent()));
      SmallPtrSet<BasicBlock*, 8> Seen;
      while (!Worklist.empty() && !PhiForm) {
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
            PhiForm = &PN;
            break;
          }
        }
        if (!PhiForm)
          for (BasicBlock* S : successors(BB))
            Worklist.push_back(S);
      }
      MaxUpdatePhi = PhiForm;
    }

    // fix-5: SelectInst fallback (form b).
    if (!MaxUpdatePhi) {
      Value* LatchVal = MaxPhi->getIncomingValueForBlock(OuterLatch);
      if (auto* Sel = dyn_cast<SelectInst>(LatchVal)) {
        if (Sel->getCondition() == MaxCompare) {
          Value *TV = Sel->getTrueValue(), *FV = Sel->getFalseValue();
          if ((TV == CutLCSSA && FV == MaxPhi) ||
              (TV == MaxPhi && FV == CutLCSSA))
            MaxUpdatePhi = Sel;
        }
      }
    }
  }

  if (!MaxUpdatePhi)
    return std::nullopt;
  LLVM_DEBUG(dbgs() << "MaxUpdate found\n");

  // 2.5: Outer latch GEP advances OuterPtrPhi by sizeof(subset element).
  GetElementPtrInst* OuterLatchGEP = nullptr;
  auto* SubsetElemAlloca = dyn_cast<AllocaInst>(Inner.SubsetAlloca);
  if (!SubsetElemAlloca)
    return std::nullopt;
  Type* OuterElemTy = SubsetElemAlloca->getAllocatedType();
  const DataLayout& DL = OuterHeader->getModule()->getDataLayout();
  uint64_t OuterStride = DL.getTypeAllocSize(OuterElemTy);

  for (Instruction& I : *OuterLatch) {
    auto* GEP = dyn_cast<GetElementPtrInst>(&I);
    if (!GEP || GEP->getPointerOperand() != OuterPtrPhi)
      continue;
    if (GEP->getNumIndices() != 1)
      continue;
    auto* Idx = dyn_cast<ConstantInt>(GEP->idx_begin()->get());
    if (!Idx || !Idx->equalsInt(OuterStride))
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
  LLVM_DEBUG(dbgs() << "GEP Done\n");

  // 2.6: Identify subset enumeration loops (future removal targets).
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
      LLVM_DEBUG(dbgs() << *TopL << "\n");
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
  if (M.MaxCompare)
    errs() << "    max cmp     : " << *M.MaxCompare << "\n";
  else
    errs() << "    max cmp     : (llvm.smax intrinsic)\n";
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
    LLVM_DEBUG(dbgs() << "[MaxCut-CPP] scanning: " << F.getName() << "\n");

    SmallVector<MaxCutMatch, 2> Matches;
    SmallVector<Loop*, 16> AllLoops;
    for (Loop* TopL : LI)
      collectAllLoops(TopL, AllLoops);

    LLVM_DEBUG(dbgs() << AllLoops.size() << " loops\n");
    for (Loop* L : AllLoops) {
      auto Scoring = matchScoringLoop(L);
      if (!Scoring)
        continue;
      LLVM_DEBUG(dbgs() << "Found Scoring Loop\n");

      auto Full = matchMaxCut(*Scoring, LI);
      if (!Full) {
        LLVM_DEBUG(dbgs() << "  [note] scoring loop in "
                          << L->getHeader()->getName()
                          << " has no enclosing max-tracking loop\n");
        continue;
      }

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
      LLVM_DEBUG(dbgs() << "  no MaxCut-CPP pattern found.\n");
    else
      for (auto& M : Matches)
        printMatch(M);

    return PreservedAnalyses::all();
  }
};
}  // namespace

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
