#include "llvm/Analysis/IVDescriptors.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

void printBlock(BasicBlock* BB, raw_ostream& OS) {
  if (!BB)
    OS << "<none>";
  else
    BB->printAsOperand(OS, false);
}

struct MinPass : PassInfoMixin<MinPass> {
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    LoopInfo& LI = AM.getResult<LoopAnalysis>(F);
    ScalarEvolution& SE = AM.getResult<ScalarEvolutionAnalysis>(F);

    errs() << "Hello FirstpASS, function: " << F.getName() << "\n";
    errs() << "  Top Level Loops: " << LI.getTopLevelLoops().size() << "\n";

    for (Loop* L : LI) {
      // clang-format off
      errs() << "Header: "; printBlock(L->getHeader(), errs()); errs() << "\n";
      errs() << "  Preheader: "; printBlock(L->getLoopPreheader(), errs()); errs() << "\n";
      errs() << "  Latch: "; printBlock(L->getLoopLatch(), errs()); errs() << "\n";
      // clang-format on

      PHINode* IndVar = L->getInductionVariable(SE);
      if (IndVar) {
        errs() << "  Induction Var: " << *IndVar << "\n";
        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(IndVar, L, &SE, ID)) {
          if (Instruction* Step = ID.getInductionBinOp()) {
            errs() << "  Increment: " << *Step << "\n";
          } else {
            errs() << "  Increment: <not a simple binop step\n";
          }
        }
      } else {
        errs() << "  Induction Var: <none>\n";
      }

      for (PHINode& PN : L->getHeader()->phis()) {
        errs() << "  Checking phi: " << PN << "\n";
        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(&PN, L, &SE, ID)) {
          errs() << "    -> IS an induction variable\n";
          if (Instruction* Step = ID.getInductionBinOp())
            errs() << "    -> step: " << *Step << "\n";
        } else {
          errs() << "    -> not recognized as induction\n";
        }
      }

      if (BasicBlock* Latch = L->getLoopLatch()) {
        if (auto* BI = dyn_cast<CondBrInst>(Latch->getTerminator()))
          errs() << "  Latch cond: " << *BI->getCondition() << "\n";
        else
          errs() << "  Latch cond: <unconditional latch>\n";
      }

      if (auto* BI = dyn_cast<CondBrInst>(L->getHeader()->getTerminator()))
        errs() << "  Header cond: " << *BI->getCondition() << "\n";
      else
        errs() << "  Header cond: <unconditional latch>\n";
    }

    return PreservedAnalyses::all();
  }
};

}  // namespace

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
          }};
}
