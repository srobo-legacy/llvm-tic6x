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
  class AlignmentFixing : public FunctionPass {
  private:
    const TargetData* TD;
    typedef ValueMap<Value*, unsigned> AlignmentMap;
    AlignmentMap KnownAlignments;
  public:
    static char ID;
    AlignmentFixing() : FunctionPass(&ID), TD(0) {}

    unsigned minimumAlignmentOfValue(Value &V);
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

  unsigned smallerAlignment(unsigned A, unsigned B) {
    if (A == 0) return B;
    if (B == 0) return A;
    return std::min(A, B);
  }
}

unsigned AlignmentFixing::minimumAlignmentOfValue(Value &V) {
  // check if we have a known alignment
  AlignmentMap::iterator AI = KnownAlignments.find(&V);
  if (AI != KnownAlignments.end())
    return AI->second;
  // mark as 1-byte aligned, in case of some recursion death
  KnownAlignments.insert(std::make_pair(&V, 1));
  // calculate properly
  unsigned Alignment;
  // here we go!
  if (GlobalValue *GV = dyn_cast<GlobalValue>(&V))
    Alignment = GV->getAlignment();
  else if (AllocaInst *AI = dyn_cast<AllocaInst>(&V))
    Alignment = AI->getAlignment();
  else if (SelectInst *SI = dyn_cast<SelectInst>(&V))
    Alignment = smallerAlignment(minimumAlignmentOfValue(*SI->getTrueValue()),
                                 minimumAlignmentOfValue(*SI->getFalseValue()));
  else if (PHINode *PN = dyn_cast<PHINode>(&V)) {
    Alignment = 0;
    for (unsigned i = 0, n = PN->getNumIncomingValues(); i != n; ++i) {
      Alignment = smallerAlignment(Alignment,
                       minimumAlignmentOfValue(*PN->getIncomingValue(i)));
    }
  }
  else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&V)) {
    llvm::Value &Operand = *GEP->getPointerOperand();
    if (containsPackedStruct(Operand.getType()))
      Alignment = 1;
    else
      Alignment = minimumAlignmentOfValue(Operand);
  }
  else if (isa<IntToPtrInst>(&V))
    Alignment = 1;
  else if (BitCastInst *BCI = dyn_cast<BitCastInst>(&V)) {
    const PointerType *DestPtrType = cast<PointerType>(BCI->getDestTy());
    const PointerType *SrcPtrType  = cast<PointerType>(BCI->getSrcTy());
    const Type *DestType = DestPtrType->getElementType();
    const Type *SrcType  = SrcPtrType->getElementType();
    if (!TD)
      Alignment = 1;
    unsigned DestAlign = TD->getABITypeAlignment(DestType);
    unsigned SrcAlign  = TD->getABITypeAlignment(SrcType);
    if (SrcAlign < DestAlign)
      Alignment = smallerAlignment(SrcAlign,
                                minimumAlignmentOfValue(*BCI->getOperand(0)));
    else
      Alignment = minimumAlignmentOfValue(*BCI->getOperand(0));
  }
  else if (isa<Argument>(&V))
    Alignment = 1; // be pessimistic, for now
  else if (isa<LoadInst>(&V))
    Alignment = 1; // again, pessimism, lots of it!
  else
    Alignment = 0;
  KnownAlignments.insert(std::make_pair(&V, Alignment));
  return Alignment;
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

bool AlignmentFixing::runOnFunction(Function &F) {
  TD = getAnalysisIfAvailable<TargetData>();
  bool MadeChange = false;
  for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
    BasicBlock &BB = *FI;
    for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
      if (BI->getOpcode() == Instruction::Load ||
          BI->getOpcode() == Instruction::Store)
        MadeChange |= fixMemoryInstruction(*BI);
  }
  return MadeChange;
}
