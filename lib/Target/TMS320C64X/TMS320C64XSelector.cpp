//===---- TMS320C64XSelector.cpp - Instruction selector for TMS320C64X ----===//
//
// Copyright 2010 Jeremy Morse <jeremy.morse@gmail.com>. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JEREMY MORSE ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL JEREMY MORSE OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
	MemSDNode *mem;
	ConstantSDNode *CN;
	unsigned int align, want_align;
	int offset;

	if (N.getOpcode() == ISD::FrameIndex)
		return false;

	if (N.getOpcode() == ISD::TargetExternalSymbol ||
				N.getOpcode() == ISD::TargetGlobalAddress)
		return false;

	mem = dyn_cast<MemSDNode>(op);
	if (mem == NULL)
		return false;

	align = mem->getAlignment();
	if (!mem->getMemoryVT().isInteger()) {
		llvm_unreachable("Memory access on non integer type\n");
	} else if (!mem->getMemoryVT().isSimple()) {
		llvm_unreachable("Memory access on non-simple type\n");
	} else {
		want_align = mem->getMemoryVT().getSimpleVT().getSizeInBits();
		want_align /= 8;

		if (align < want_align) {
			fprintf(stderr, "knobbling nonalign access\n");
			return false;
		}
	}

	if (N.getOperand(0).getOpcode() == ISD::Register &&
		N.getOperand(1).getOpcode() == ISD::Constant) {
		if ((N.getOpcode() == ISD::ADD || N.getOpcode() == ISD::SUB) &&
				(Predicate_sconst_n(N.getOperand(1).getNode(),
							want_align + 5))) {

			// This is valid in a single instruction. Offset operand
			// will be analysed by asm printer to detect the correct
			// addressing mode to print. The assembler will scale
			// the constant appropriately.
			CN = cast<ConstantSDNode>(N.getOperand(1));
			offset = CN->getSExtValue();

			if (N.getOpcode() == ISD::SUB)
				offset = -offset;

			base = N.getOperand(0);
			offs = CurDAG->getTargetConstant(offset, MVT::i32);
			return true;
		} else if (N.getOpcode() == ISD::ADD ||
						N.getOpcode() == ISD::SUB) {
			// Too big - load into register. Because the processor
			// scales the offset, even when its being used as an
			// offset in a register, we need to shift what gets
			// loaded at this point.
			base = N.getOperand(0);
			CN = cast<ConstantSDNode>(N.getOperand(1));
			offset = CN->getSExtValue();

			if (offset & ((1 << want_align) - 1)) {
				// Offset doesn't honour alignment rules.
				// Ideally we should now morph to using a
				// nonaligned memory instruction, but for now
				// leave this as unsupported
				fprintf(stderr, "jmorse: unaligned offset to "
					"memory access, implement swapping to "
					"nonaligned instructions\n");
				return false;
			}

			// scale offset by amount hardware will
			offset >>= want_align;
			DebugLoc dl = DebugLoc::getUnknownLoc();
			MachineFunction &MF = CurDAG->getMachineFunction();
			MachineRegisterInfo &MR = MF.getRegInfo();
			unsigned reg = MR.createVirtualRegister(
					&TMS320C64X::GPRegsRegClass);

			offs = CurDAG->getCopyToReg(N.getOperand(1), dl, reg,
					CurDAG->getTargetConstant(offset,
					MVT::i32));
		} else {
			return false;
		}
	} else if (N.getOperand(0).getOpcode() == ISD::Register &&
		N.getOperand(1).getOpcode() == ISD::Register) {
		// We can use operand as index if it's add - just leave
		// as 2nd operand
		if (N.getOpcode() != ISD::ADD)
			return false;
	} else {
		return false;
	}

	/* Ok, we have a memory reference in range. However its scaled by the
	 * size of the data access before being used (XXX - fix size tests above
	 * to handle this). So, insert a SHR on the offset: if a constant, fine,
	 * if a register, it'll have to be shr'd at runtime */
	DebugLoc dl = DebugLoc::getUnknownLoc();
	offs = CurDAG->getNode(ISD::SRA, dl, MVT::i32, offs,
			CurDAG->getTargetConstant(log2(align), MVT::i32));
	offs = SDValue(SelectCode(offs), 0);
	return true;
}

bool
TMS320C64XInstSelectorPass::select_idxaddr(SDValue op, SDValue addr,
					SDValue &base, SDValue &offs)
{
	MemSDNode *mem;
	FrameIndexSDNode *FIN;
	unsigned int align, want_align;

	mem = dyn_cast<MemSDNode>(op);
	if (mem == NULL)
		return false;

	align = mem->getAlignment();
	if (!mem->getMemoryVT().isInteger()) {
		llvm_unreachable("Memory access on non integer type\n");
	} else if (!mem->getMemoryVT().isSimple()) {
		llvm_unreachable("Memory access on non-simple type\n");
	} else {
		want_align = mem->getMemoryVT().getSimpleVT().getSizeInBits();
		want_align /= 8;

		if (align < want_align) {
			fprintf(stderr, "knobbling nonalign access\n");
			return false;
		}
	}

	FIN = dyn_cast<FrameIndexSDNode>(addr);
	if (FIN) {
		base = CurDAG->getRegister(TMS320C64X::A15, MVT::i32);
		offs = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
	} else if (addr.getOpcode() == ISD::TargetExternalSymbol ||
				addr.getOpcode() == ISD::TargetGlobalAddress) {
		return false;
	} else if (addr.getOpcode() == ISD::ADD) {
		// We could match against the proper indexed load things here,
		// and emit a single instruction for loading, but that can be
		// implemented at some other point in time
		// XXX - death
	} else {
		base = addr;
		offs = CurDAG->getTargetConstant(0, MVT::i32);
	}

	/* See comment in select_addr */
	DebugLoc dl = DebugLoc::getUnknownLoc();
	offs = CurDAG->getNode(ISD::SRA, dl, MVT::i32, offs,
			CurDAG->getTargetConstant(log2(align), MVT::i32));
	offs = SDValue(SelectCode(offs), 0);
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
