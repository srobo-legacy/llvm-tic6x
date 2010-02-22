//===---- TMS320C64XSelector.cpp - Instruction selector for TMS320C64X ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "TMS320C64XTargetMachine.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Intrinsics.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
class TMS320C64XInstSelectorPass : public SelectionDAGISel {
public:
	explicit TMS320C64XInstSelectorPass(TargetMachine &TM);

	virtual void InstructionSelect();
	SDNode *Select(SDValue op);
	bool select_addr(SDValue op, SDValue N, SDValue &R1, SDValue &R2);
	bool select_idxaddr(SDValue op, SDValue N, SDValue &R1, SDValue &R2);
	const char *getPassName() const {
		return "TMS320C64X Instruction Selection";
	}

#include "TMS320C64XGenDAGISel.inc"
};
}

FunctionPass*
llvm::TMS320C64XCreateInstSelector(TargetMachine &TM)
{

	return new TMS320C64XInstSelectorPass(TM);
}

TMS320C64XInstSelectorPass::TMS320C64XInstSelectorPass(TargetMachine &TM)
	: SelectionDAGISel(TM)
{
}

void
TMS320C64XInstSelectorPass::InstructionSelect()
{

	SelectRoot(*CurDAG);
	CurDAG->RemoveDeadNodes();
}

bool
TMS320C64XInstSelectorPass::select_addr(SDValue op, SDValue N, SDValue &R1,
					SDValue &R2)
{

	// Bees
	return false;
}

bool
TMS320C64XInstSelectorPass::select_idxaddr(SDValue op, SDValue addr,
					SDValue &base, SDValue &offs)
{
	FrameIndexSDNode *FIN;

	FIN = dyn_cast<FrameIndexSDNode>(addr);
	if (FIN) {
		base = CurDAG->getRegister(TMS320C64X::A15, MVT::i32);
		offs = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
		return true;
	}

	if (addr.getOpcode() == ISD::TargetExternalSymbol ||
				addr.getOpcode() == ISD::TargetGlobalAddress)
		return false;

	if (addr.getOpcode() == ISD::ADD) {
		// We could match against the proper indexed load things here,
		// and emit a single instruction for loading, but that can be
		// implemented at some other point in time
		// XXX - death
	}

	base = addr;
	offs = CurDAG->getTargetConstant(0, MVT::i32);
	return true;
}

SDNode *
TMS320C64XInstSelectorPass::Select(SDValue op)
{

	return SelectCode(op);
}
