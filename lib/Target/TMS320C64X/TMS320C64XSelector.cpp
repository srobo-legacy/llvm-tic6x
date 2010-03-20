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
#include "TMS320C64XRegisterInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Intrinsics.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
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
	bool bounce_predicate(SDValue op, SDValue N, SDValue &R1, SDValue &R2);
	const char *getPassName() const {
		return "TMS320C64X Instruction Selection";
	}

// What the fail.
#define UNKNOWN MVT::i32
#include "TMS320C64XGenDAGISel.inc"
#undef UNKNOWN
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
TMS320C64XInstSelectorPass::select_addr(SDValue op, SDValue N, SDValue &base,
					SDValue &offs)
{
	if (N.getOpcode() == ISD::FrameIndex)
		return false;

	if (N.getOpcode() == ISD::TargetExternalSymbol ||
				N.getOpcode() == ISD::TargetGlobalAddress)
		return false;

	if (N.getOperand(0).getOpcode() == ISD::Register &&
		N.getOperand(1).getOpcode() == ISD::Constant) {
		if (N.getOpcode() == ISD::ADD &&
				(Predicate_uconst5(N.getOperand(1).getNode()) ||
				Predicate_uconst15(N.getOperand(1).getNode()))){
			// This is valid, we can just print it
			base = N.getOperand(0);
			offs = N.getOperand(1);
			return true;
		} else if (N.getOpcode() == ISD::ADD ||
						N.getOpcode() == ISD::SUB) {
			// Too big - load into register
			base = N.getOperand(0);

			DebugLoc dl = DebugLoc::getUnknownLoc();
			MachineFunction &MF = CurDAG->getMachineFunction();
			MachineRegisterInfo &MR = MF.getRegInfo();
			unsigned reg = MR.createVirtualRegister(
					&TMS320C64X::GPRegsRegClass);
			// XXX - damned if I know what chain is supposed
			// to be in this situation
			offs = CurDAG->getCopyToReg(N.getOperand(1),
					dl, reg, N.getOperand(1));
			return true;
		} else {
			return false;
		}
	} else if (N.getOperand(0).getOpcode() == ISD::Register &&
		N.getOperand(1).getOpcode() == ISD::Constant) {
		// We can use operand as index if it's add - just leave
		// as 2nd operand
		if (N.getOpcode() != ISD::ADD)
			return false;
		else
			return true;
	}

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

bool
TMS320C64XInstSelectorPass::bounce_predicate(SDValue op, SDValue N, SDValue
							&base, SDValue &offs)
{
	int sz;

	// We assume that whoever generated this knew what they were doing,
	// and that they've placed the predecate operands in the last two
	// operand positions. So just return those.
	sz = op.getNumOperands();
	base = op.getOperand(sz-2);
	offs = op.getOperand(sz-1);
	return true;
}

SDNode *
TMS320C64XInstSelectorPass::Select(SDValue op)
{

	return SelectCode(op);
}
