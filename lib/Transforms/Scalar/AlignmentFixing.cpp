#define DEBUG_TYPE "alignment-fixing"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ValueHandle.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Target/TargetData.h"
#include <algorithm>
using namespace llvm;

STATISTIC(NumFixAlign, "Number of alignments fixed");

namespace {
  class AlignmentFixing : public ModulePass {
  private:
    const TargetData* TD;
    //ValueMap<unsigned> KnownAlignments;
  public:
    static char ID;
    AlignmentFixing() : ModulePass(&ID), TD(0) {}

    unsigned minimumAlignmentOfValue(Value &V);
    bool mayBeUnaligned(Value &V);
    bool fixMemoryInstruction(Instruction &I);
    bool runOnModule(Module &M);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }
  };
}

char AlignmentFixing::ID = 0;
static RegisterPass<AlignmentFixing> X("alignment-fix", "Fix alignments");

// Public interface to the AlignmentFixing pass
Pass *llvm::createAlignmentFixingPass() {
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

unsigned AlignmentFixing::minimumAlignmentOfValue(Value &V) {
  // TODO: fill in properly
  return mayBeUnaligned(V) ? 1 : 0;
}

bool AlignmentFixing::fixMemoryInstruction(Instruction &I) {
  llvm::Value* Target;
  bool MadeChange = false;
  if (I.getOpcode() == Instruction::Load)
    Target = I.getOperand(0);
  else
    Target = I.getOperand(1);
  unsigned minAlignment = minimumAlignmentOfValue(*Target);
  if (minAlignment > 0) {
    if (I.getOpcode() == Instruction::Load) {
      LoadInst& LI = *cast<LoadInst>(&I);
      if (LI.getAlignment() > minAlignment ||
          LI.getAlignment() == 0) {
        LI.setAlignment(minAlignment);
        MadeChange = true;
      }
    }
    else if (I.getOpcode() == Instruction::Store) {
      StoreInst& SI = *cast<StoreInst>(&I);
      if (SI.getAlignment() > minAlignment ||
          SI.getAlignment() == 0) {
        SI.setAlignment(minAlignment);
        MadeChange = true;
      }
    }
    else
      assert(0 && "giant death cakes of deathly death");
    ++NumFixAlign;
  }
  if (MadeChange)
    ++NumFixAlign;
  return MadeChange;
}

bool AlignmentFixing::runOnModule(Module &M) {
  TD = getAnalysisIfAvailable<TargetData>();
  bool MadeChange = false;
  for (Module::iterator MI = M.begin(), ME = M.end(); MI != ME; ++MI) {
    Function &F = *MI;
    for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
      BasicBlock &BB = *FI;
      for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
        if (BI->getOpcode() == Instruction::Load ||
            BI->getOpcode() == Instruction::Store) {
          MadeChange |= fixMemoryInstruction(*BI);
        }
    }
  }
  return MadeChange;
}
