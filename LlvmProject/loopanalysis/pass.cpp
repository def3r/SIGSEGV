#include <optional>
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct CutEdgeMatch {
  Value* EdgesBase = nullptr;
  Value* PartitionBase = nullptr;
  PHINode* Accumulator = nullptr;
  ICmpInst* Compare = nullptr;
};

Value* matchLoadFromInductionIndexedGEP(Value* V,
                                        PHINode* IndVar,
                                        Value*& OutIndex) {
  auto* Load = dyn_cast<LoadInst>(V);
  if (!Load)
    return nullptr;

  auto* GEP = dyn_cast<GetElementPtrInst>(Load->getPointerOperand());
  if (!GEP)
    return nullptr;

  OutIndex = Load;

  // Shape A: GEP directly indexed by IndVar (e.g. edges[i][0]).
  for (Value* Idx : GEP->indices()) {
    if (Idx == IndVar)
      return GEP->getPointerOperand();
  }

  // Shape B: GEP's base pointer is itself a GEP indexed by IndVar, and
  // this GEP just adds a constant byte offset (e.g. edges[i][1], which
  // clang lowers as "row-i-pointer + 4 bytes" rather than re-indexing).
  if (auto* InnerGEP = dyn_cast<GetElementPtrInst>(GEP->getPointerOperand())) {
    for (Value* Idx : InnerGEP->indices()) {
      if (Idx == IndVar)
        return InnerGEP->getPointerOperand();
    }
  }

  return nullptr;
}

std::optional<CutEdgeMatch> matchCutEdgeCountingLoop(Loop* L,
                                                     ScalarEvolution& SE) {
  CutEdgeMatch Result;

  // 1. Induction variable.
  PHINode* IndVar = nullptr;
  for (PHINode& PN : L->getHeader()->phis()) {
    InductionDescriptor ID;
    if (InductionDescriptor::isInductionPHI(&PN, L, &SE, ID)) {
      IndVar = &PN;
      break;
    }
  }
  if (!IndVar)
    return std::nullopt;

  // 2. Find the compare: load(GEP(Partition, load(GEP(Edges, IndVar))))
  //    on both sides.
  for (BasicBlock* BB : L->getBlocks()) {
    for (Instruction& I : *BB) {
      auto* Cmp = dyn_cast<ICmpInst>(&I);
      if (!Cmp)
        continue;

      errs() << "  [debug] checking icmp: " << *Cmp << "\n";

      Value* LHS = Cmp->getOperand(0);
      Value* RHS = Cmp->getOperand(1);

      auto* LHSLoad = dyn_cast<LoadInst>(LHS);
      auto* RHSLoad = dyn_cast<LoadInst>(RHS);
      if (!LHSLoad || !RHSLoad) {
        errs() << "    [debug] FAIL: LHS or RHS not a LoadInst\n";
        continue;
      }

      auto* LHSGEP = dyn_cast<GetElementPtrInst>(LHSLoad->getPointerOperand());
      auto* RHSGEP = dyn_cast<GetElementPtrInst>(RHSLoad->getPointerOperand());
      if (!LHSGEP || !RHSGEP) {
        errs() << "    [debug] FAIL: LHS or RHS load-pointer not a GEP\n";
        continue;
      }

      if (LHSGEP->getPointerOperand() != RHSGEP->getPointerOperand()) {
        errs() << "    [debug] FAIL: partition base pointers differ: "
               << *LHSGEP->getPointerOperand() << "  vs  "
               << *RHSGEP->getPointerOperand() << "\n";
        continue;
      }

      auto resolveIndexSource = [&](GetElementPtrInst* GEP) -> Value* {
        Value* IdxOperand = GEP->getOperand(GEP->getNumOperands() - 1);
        errs() << "    [debug] raw index operand: " << *IdxOperand << "\n";
        if (auto* Cast = dyn_cast<CastInst>(IdxOperand)) {
          IdxOperand = Cast->getOperand(0);
          errs() << "    [debug] stripped cast, now: " << *IdxOperand << "\n";
        }
        Value* Unused;
        Value* Base =
            matchLoadFromInductionIndexedGEP(IdxOperand, IndVar, Unused);
        errs() << "    [debug] resolved edges base: "
               << (Base ? "found" : "NULL") << "\n";
        return Base;
      };

      Value* EdgesBaseL = resolveIndexSource(LHSGEP);
      Value* EdgesBaseR = resolveIndexSource(RHSGEP);
      if (!EdgesBaseL || !EdgesBaseR || EdgesBaseL != EdgesBaseR) {
        errs() << "    [debug] FAIL: edges base mismatch or null\n";
        continue;
      }

      Result.EdgesBase = EdgesBaseL;
      Result.PartitionBase = LHSGEP->getPointerOperand();
      Result.Compare = Cmp;
      goto found_compare;
    }
  }
  return std::nullopt;

found_compare:
  // 3. Accumulator: phi whose backedge value is add(phi, zext(Compare)).
  {
    BasicBlock* Latch = L->getLoopLatch();
    if (!Latch)
      return std::nullopt;

    for (PHINode& PN : L->getHeader()->phis()) {
      if (&PN == IndVar)
        continue;

      Value* BackedgeVal = PN.getIncomingValueForBlock(Latch);
      auto* Add = dyn_cast<BinaryOperator>(BackedgeVal);
      if (!Add || Add->getOpcode() != Instruction::Add)
        continue;

      Value *Op0 = Add->getOperand(0), *Op1 = Add->getOperand(1);
      Value* OtherOperand = nullptr;
      if (Op0 == &PN)
        OtherOperand = Op1;
      else if (Op1 == &PN)
        OtherOperand = Op0;
      else
        continue;

      auto* Zext = dyn_cast<ZExtInst>(OtherOperand);
      if (!Zext || Zext->getOperand(0) != Result.Compare)
        continue;

      Result.Accumulator = &PN;
      return Result;
    }
  }
  return std::nullopt;
}

struct MinPass : PassInfoMixin<MinPass> {
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    LoopInfo& LI = AM.getResult<LoopAnalysis>(F);
    ScalarEvolution& SE = AM.getResult<ScalarEvolutionAnalysis>(F);

    errs() << "Function: " << F.getName() << "\n";

    for (Loop* L : LI) {
      auto Match = matchCutEdgeCountingLoop(L, SE);
      if (Match) {
        errs() << "  MATCHED max-cut / cut-edge-counting pattern:\n";
        errs() << "    Edges base:     " << *Match->EdgesBase << "\n";
        errs() << "    Partition base: " << *Match->PartitionBase << "\n";
        errs() << "    Compare:        " << *Match->Compare << "\n";
        errs() << "    Accumulator:    " << *Match->Accumulator << "\n";
      } else {
        errs() << "  No match for this loop.\n";
      }
    }

    return PreservedAnalyses::all();
  }
};

}  // namespace

void registerMaxCutCppPass(PassBuilder& PB);

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MinPass", LLVM_VERSION_STRING,
          [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "min-pass") {
                    FPM.addPass(MinPass());
                    return true;
                  }
                  return false;
                });
            registerMaxCutCppPass(PB);
          }};
}
