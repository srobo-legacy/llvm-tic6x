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
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

namespace {
class TMS320C64XInstSelectorPass : public SelectionDAGISel {
public:
	explicit TMS320C64XInstSelectorPass(TargetMachine &TM);

	SDNode *Select(SDNode *op);
	bool select_addr(SDNode *&op, SDValue &N, SDValue &R1, SDValue &R2);
	bool select_idxaddr(SDNode *&op, SDValue &N, SDValue &R1, SDValue &R2);
	bool bounce_predicate(SDNode *&op, SDValue &N, SDValue &R1);

	SDNode *get_memaccess_data_reg(SDNode &op);

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

SDNode *
TMS320C64XInstSelectorPass::get_memaccess_data_reg(SDNode &op)
{

	// In a number of circumstances we need to retrieve the data register
	// from a load or store operation, if the instruction lowering phase
	// has emitted a node with a fixed data register (IE, we're writing
	// to a function argument register). This affects what load/store
	// instruction gets selected, as which data path is used is significant

	if (op.getOpcode() == ISD::STORE) {
		if (op.getOperand(0).getOpcode() == ISD::Register) {
			return op.getOperand(0).getNode();
		} else if (op.getOperand(0).getOpcode() == ISD::CopyFromReg &&
			op.getOperand(0).getNode()->getOperand(1).getOpcode()
			== ISD::Register) {
			return op.getOperand(0).getNode()->
				getOperand(1).getNode();
		}

		return NULL;
	} else if (op.getOpcode() == ISD::LOAD) {
		// Oh. The Horror.
		// So, we can derive the address-calculating operands just fine
		// from this sdnode... however, we can't do the same for the
		// result due to this being a DAG: the result is how this is
		// used, not what its operands are.
		// So we have to prod the uses to find out what register we
		// may or may not be being written to. Yeouch.

		// And that breaks down if we have more than one use
		if (!op.hasOneUse())
			return NULL;

		SDNode::use_iterator i = op.use_begin();
		SDNode *user = *i;

		if (user->getOpcode() == ISD::CopyToReg &&
			user->getOperand(1)->getOpcode() == ISD::Register)
			return user->getOperand(1).getNode();


		return NULL;
	} else {
		llvm_unreachable("Trying to extract data register from node "
				"that is neither a load nor a store");
	}
}

bool
TMS320C64XInstSelectorPass::select_addr(SDNode *&op, SDValue &N,
					SDValue &base, SDValue &offs)
{
	MemSDNode *mem;
	ConstantSDNode *CN;
	unsigned int align, want_align;
	int offset;

	DebugLoc dl = DebugLoc::getUnknownLoc();

	// We handle all memory access in this save frame index'd accesses,
	// so bounce those to select_idxaddr
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
			llvm_unreachable("Insufficient alignment on "
					"memory access");
		}
	}

	if (N.getNode()->getNumOperands() == 0 ||
		N.getOperand(0).getOpcode() == ISD::TargetGlobalAddress ||
		N.getOperand(0).getOpcode() == ISD::TargetExternalSymbol) {
		// Node is a constant (no other operands), OR,
		// Operand 0 is a global address, the node itself is a TMSISD
		// wrapper which'll get lowered into a mvkl/mvkh pair later.
		// So, we can allow the raw node to become the base address
		// safely.
		base = N;
		offs = CurDAG->getTargetConstant(0, MVT::i32);
		return true;
	} else if (N.getNumOperands() == 1) {
		// Something unpleasent - leave addr as it is, 0 offset
		base = N.getOperand(0);
		offs = CurDAG->getTargetConstant(0, MVT::i32);
		return true;
	}

	if (N.getOperand(1).getOpcode() == ISD::Constant &&
		(N.getOpcode() == ISD::ADD || N.getOpcode() == ISD::SUB)) {
		if (Predicate_uconst_n(N.getOperand(1).getNode(),
					(int)log2(want_align) + 5)) {

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
		} else if (N.getOpcode() == ISD::ADD &&
				Predicate_const_is_positive(
				N.getOperand(1).getNode()),
				Predicate_uconst_n(N.getOperand(1).getNode(),
				((int)log2(want_align)) + 15)) {
			// We can use the uconst15 form of this instruction.
			// Again, the assembler will scale this for us. Could
			// put in some clauses to find sub insns with negative
			// offsets, but I guess llvm might do that for us.
			base = N.getOperand(0);
			offs = N.getOperand(1);

			return true;
		} else {
			// Too big - load into register. Because the processor
			// scales the offset, even when its being used as an
			// offset in a register, we need to shift what gets
			// loaded at this point.
			base = N.getOperand(0);
			CN = cast<ConstantSDNode>(N.getOperand(1));
			offset = CN->getSExtValue();

			if (offset & ((1 << (int)log2(want_align)) - 1)) {
				// Offset doesn't honour alignment rules.
				// Ideally we should now morph to using a
				// nonaligned memory instruction, but for now
				// leave this as unsupported
				llvm_unreachable("jmorse: unaligned offset to "
					"memory access, implement swapping to "
					"nonaligned instructions\n");
				return false;
			}

			// XXX - there was some code here that attempted to load
			// the offs operand into a register and then pump it out
			// however this wasn't valid - LLVM doesn't expect new
			// SDNodes to occur as a result of selection. Removing
			// this should have caused some offset-too-large
			// complaints from the assembler.. but didn't. So I can
			// only assume that this is munging offsets into a
			// register or otherwise doing something unexpected. In
			// either case, memory access beating needs to be
			// rewritten, again.
			offs = N.getOperand(1);
			return true;
		}
	} else if (N.getOpcode() == ISD::ADD) {
		// No constant offset, so values will be in registers when
		// the get to us. XXX: is operand(1) always the constant, or
		// can it be in 0 too?

		// We can use operand as index if it's add - just leave
		// as 2nd operand. Could also implement allowing subtract,
		// but this means passing addressing mode information down
		// to the assembly printer, which I suspect will mean pain.

		// As mentioned above though, hardware will scale the offset,
		// so we need to insert a shift here.
		base = N.getOperand(0);
		SDValue ops[4];
		ops[0] = N.getOperand(1);
		ops[1] = CurDAG->getTargetConstant((int)log2(align), MVT::i32);
		ops[2] = CurDAG->getTargetConstant(-1, MVT::i32);
		ops[3] = CurDAG->getRegister(TMS320C64X::NoRegister, MVT::i32);
		offs = CurDAG->getNode(ISD::SRA, dl, MVT::i32, ops, 4);

		// That's a MI instruction and we're in the middle of depth
		// first instruction selection, this won't get selected. So,
		// make that happen manually.
		offs = SDValue(SelectCode(offs.getNode()), 0);
		return true;
	} else {
		// Doesn't match anything we recognize at all, use address
		// as it is (aka let llvm deal with it), set offset to zero
		// to ensure it doesn't intefere with address calculation.
		base = N;
		offs = CurDAG->getTargetConstant(0, MVT::i32);
		return true;
	}

	// Initially concerning that all of the above return true - however this
	// is after all the address selection code, and anything is valid for
	// an address, all we're doing here is shortening the calculations for
	// some forms.
}

bool
TMS320C64XInstSelectorPass::select_idxaddr(SDNode *&op, SDValue &addr,
					SDValue &base, SDValue &offs)
{
	MemSDNode *mem;
	FrameIndexSDNode *FIN;
	unsigned int align, want_align;

	if (op->getOpcode() == ISD::FrameIndex) {
		// Hackity hack: llvm wants the address of a stack slot. This
		// is handled by returning the frame pointer as base and stack
		// offset as offs; the "lea_fail" instruction then adds these
		// to form a pointer.
		base = CurDAG->getRegister(TMS320C64X::A15, MVT::i32);
		FIN = cast<FrameIndexSDNode>(op);
		offs = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
		return true;
	}

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
			llvm_unreachable("Insufficient alignment on "
					"memory access");
		}
	}

	FIN = dyn_cast<FrameIndexSDNode>(addr);
	if (!FIN)
		return false;

	base = CurDAG->getRegister(TMS320C64X::A15, MVT::i32);
	offs = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);

	return true;
}

bool
TMS320C64XInstSelectorPass::bounce_predicate(SDNode *&op, SDValue &N, SDValue
								&out)
{
	int sz;

	// We assume that whoever generated this knew what they were doing,
	// and that they've placed the predecate operands in the last
	// operand position. So just return those.
	sz = op->getNumOperands();
	out = op->getOperand(sz-1);
	return true;
}

SDNode *
TMS320C64XInstSelectorPass::Select(SDNode *op)
{

	return SelectCode(op);
}
