#define DEBUG_TYPE "alignment-fixing"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ValueHandle.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include <algorithm>
using namespace llvm;

STATISTIC(NumFixAlign, "Number of alignments fixed");

namespace {
  class AlignmentFixing : public FunctionPass {
  public:
    static char ID;
    AlignmentFixing() : FunctionPass(&ID) {}

    bool mayBeUnaligned(Value &V);
    bool fixMemoryInstruction(Instruction &I);
    bool runOnFunction(Function &F);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }
  };
}

char AlignmentFixing::ID = 0;
static RegisterPass<AlignmentFixing> X("alignment-fix", "Fix alignments");

// Public interface to the AlignmentFixing pass
FunctionPass *llvm::createAlignmentFixingPass() {
  return new AlignmentFixing();
}

namespace {
  bool containsPackedStruct(const Type *T) {
    if (const SequentialType *ST = dyn_cast<SequentialType>(T))
      return containsPackedStruct(ST->getElementType());
    if (const StructType *ST = dyn_cast<StructType>(T)) {
      if (ST->isPacked())
        return true;
      for (unsigned i = 0, e = ST->getNumElements(); i != e; ++i) {
        if (containsPackedStruct(ST->getTypeAtIndex(i)))
          return true;
      }
    }
    return false;
  }
}

bool AlignmentFixing::mayBeUnaligned(Value &V) {
  if (GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(&V)) {
    llvm::Value& Operand = *GEPI->getPointerOperand();
    return containsPackedStruct(Operand.getType()) ||
           mayBeUnaligned(Operand);
  }
  return false;
}

bool AlignmentFixing::fixMemoryInstruction(Instruction &I) {
  bool potentialUnalignment = mayBeUnaligned(*(I.getOperand(0)));
  if (potentialUnalignment) {
    if (I.getOpcode() == Instruction::Load)
      cast<LoadInst>(&I)->setAlignment(1);
    else if (I.getOpcode() == Instruction::Store)
      cast<StoreInst>(&I)->setAlignment(1);
    else
      assert(0 && "giant death cakes of deathly death");
    ++NumFixAlign;
    return true;
  }
  return false;
}

bool AlignmentFixing::runOnFunction(Function &F) {
  bool MadeChange = false;
  for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
    BasicBlock &BB = *FI;
    for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
      if (BI->getOpcode() == Instruction::Load ||
          BI->getOpcode() == Instruction::Store) {
        MadeChange |= fixMemoryInstruction(*BI);
      }
  }
  return MadeChange;
}
