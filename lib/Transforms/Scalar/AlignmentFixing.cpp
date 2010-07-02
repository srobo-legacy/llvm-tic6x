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
#include "llvm/ADT/SmallSet.h"
#include "llvm/Target/TargetData.h"
#include <algorithm>
using namespace llvm;

STATISTIC(NumFixAlign, "Number of alignments fixed");

namespace {
  class AlignmentFixing : public FunctionPass {
  private:
    const TargetData* TD;
  public:
    static char ID;
    AlignmentFixing() : FunctionPass(&ID), TD(0) {}

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
  bool containsPackedStruct(const Type *T, SmallSet<const Type*, 8>& CT) {
    if (CT.count(T)) return false;
    CT.insert(T);
    if (const SequentialType *ST = dyn_cast<SequentialType>(T))
      return containsPackedStruct(ST->getElementType(), CT);
    if (const StructType *ST = dyn_cast<StructType>(T)) {
      if (ST->isPacked())
        return true;
      for (unsigned i = 0, e = ST->getNumElements(); i != e; ++i) {
        if (containsPackedStruct(ST->getTypeAtIndex(i), CT))
          return true;
      }
    }
    return false;
  }

  bool containsPackedStruct(const Type *T) {
    SmallSet<const Type*, 8> CT;
    return containsPackedStruct(T, CT);
  }
}

bool AlignmentFixing::mayBeUnaligned(Value &V) {
  if (GetElementPtrInst* GEPI = dyn_cast<GetElementPtrInst>(&V)) {
    llvm::Value& Operand = *GEPI->getPointerOperand();
    return containsPackedStruct(Operand.getType()) ||
           mayBeUnaligned(Operand);
  } else if (BitCastInst* BCI = dyn_cast<BitCastInst>(&V)) {
    const Type* DestType = cast<PointerType>(BCI->getDestTy())->getElementType();
    const Type* SourceType = cast<PointerType>(BCI->getSrcTy())->getElementType();
    if (!TD)
      return true;
    if (TD->getABITypeAlignment(SourceType) <
        TD->getABITypeAlignment(DestType))
      return true;
    return mayBeUnaligned(*BCI->getOperand(0));
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
  TD = getAnalysisIfAvailable<TargetData>();
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
