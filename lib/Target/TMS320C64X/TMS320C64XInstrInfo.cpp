//===- TMS320C64XInstrInfo.cpp - TMS320C64X Instruction Information -------===//
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

#include "TMS320C64XInstrInfo.h"
#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XTargetMachine.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

TMS320C64XInstrInfo::TMS320C64XInstrInfo(TMS320C64XTargetMachine &tm)
  : TargetInstrInfoImpl(TMS320C64XInsts, array_lengthof(TMS320C64XInsts)),
    RI(tm, *this), TM(tm)
{

	return;
}

bool
TMS320C64XInstrInfo::spillCalleeSavedRegisters(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator MBBI,
		const std::vector<CalleeSavedInfo> &CSI) const
{
	const MachineFunction *MF;
	unsigned int i, reg;
	bool is_kill;

	MF = MBB.getParent();
	const MachineRegisterInfo &MRI = MF->getRegInfo();

	for (i = 0; i < CSI.size(); ++i) {
		// Should this be a kill? Unfortunately the argument registers
		// and nonvolatile registers on the target overlap, which leads
		// to a situation where we spill a nonvolatile register,
		// killing it, and then try and use it as an argument register
		// -> "Using undefined register". So, check whether this reg is
		// a _function_ LiveIn too.

		is_kill = true;
		reg = CSI[i].getReg();

		MachineRegisterInfo::livein_iterator li = MRI.livein_begin();
		for (; li != MRI.livein_end(); li++) {
			if (li->first == reg) {
				is_kill = false;
				break;
			}
		}

		MBB.addLiveIn(reg);
		storeRegToStackSlot(MBB, MBBI, reg, is_kill,
					CSI[i].getFrameIdx(),
					CSI[i].getRegClass());
	}

	return true;
}

bool
TMS320C64XInstrInfo::copyRegToReg(MachineBasicBlock &MBB, 
				MachineBasicBlock::iterator I,
				unsigned dst_reg, unsigned src_reg,
				const TargetRegisterClass *dst_class,
				const TargetRegisterClass *src_class) const
{

	DebugLoc DL = DebugLoc::getUnknownLoc();

	addDefaultPred(BuildMI(MBB, I, DL, get(TMS320C64X::mv))
			.addReg(dst_reg, RegState::Define).addReg(src_reg));
	return true;
}

void
TMS320C64XInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator I, unsigned src_reg,
		bool is_kill, int FI, const TargetRegisterClass *rc) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();

	addDefaultPred(BuildMI(MBB, I, DL, get(TMS320C64X::word_store1))
		.addReg(TMS320C64X::A15).addFrameIndex(FI)
		.addReg(src_reg, getKillRegState(is_kill)));
}

void
TMS320C64XInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator MI, unsigned dst_reg, int frame_idx,
		const TargetRegisterClass *rc) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();

	addDefaultPred(BuildMI(MBB, MI, DL, get(TMS320C64X::word_load1))
		.addReg(dst_reg, RegState::Define)
		.addReg(TMS320C64X::A15).addFrameIndex(frame_idx));
}

bool
TMS320C64XInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
		MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
		SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const
{
	bool predicated, saw_uncond_branch;
	int pred_idx, opcode;
	MachineBasicBlock::iterator I = MBB.end();

	saw_uncond_branch = false;

	while (I != MBB.begin()) {
		--I;

		pred_idx = I->findFirstPredOperandIdx();
		if (pred_idx == -1) {
			predicated = false;
		} else if (I->getOperand(pred_idx).getImm() != -1) {
			predicated = true;
		} else {
			predicated = false;
		}

		opcode = I->getOpcode();

		if (!predicated && (opcode == TMS320C64X::branch_p ||
					opcode == TMS320C64X::branch_1 ||
					opcode == TMS320C64X::branch_2)) {
			// We're an unconditional branch. The analysis rules
			// say that we should carry on looking, in case there's
			// a conditional branch beforehand.

			saw_uncond_branch = true;
			TBB = I->getOperand(0).getMBB();

			if (!AllowModify)
				// Nothing to be done
				continue;

			// According to what X86 does, we can delete branches
			// if they branch to the immediately following BB

			if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
				TBB = NULL;
				// FIXME - and what about trailing noops?
				I->eraseFromParent();
				I = MBB.end();
				continue;
			}

			continue;
		}

		// If we're a predicated instruction and terminate the BB,
		// we can be pretty sure we're a conditional branch
		if (predicated && (opcode == TMS320C64X::brcond_p)) {
			// Two different conditions to consider - this is the
			// only branch, in which case we fall through to the
			// next, or it's a conditional before unconditional.

			if (TBB != NULL) {
				// True condition branches to operand of
				// this conditional branch; false condition is
				// where the following unconditional goes.
				FBB = TBB;
				TBB = I->getOperand(0).getMBB();
			} else {
				TBB = I->getOperand(0).getMBB();
			}

			// Grab the condition
			Cond.push_back(I->getOperand(1)); // Zero/NZ
			Cond.push_back(I->getOperand(2)); // Reg
			return false;
		}

		// Out of branches and conditional branches, only other thing
		// we expect to see is a trailing noop

		if (opcode == TMS320C64X::noop)
			continue;

		if (saw_uncond_branch)
			// We already saw an unconditional branch, then
			// something we didn't quite understand
			return false;

		return true; // Something we don't understand at all
	}

	return false;
}

unsigned
TMS320C64XInstrInfo::InsertBranch(MachineBasicBlock &MBB,
		MachineBasicBlock *TBB, MachineBasicBlock *FBB,
		const SmallVectorImpl<MachineOperand> &Cond) const
{
	DebugLoc dl = DebugLoc::getUnknownLoc();

	assert(TBB && "InsertBranch can't insert fallthroughs");
	assert((Cond.size() == 2 || Cond.size() == 0) &&
			"Invalid condition to InsertBranch");

	if (Cond.empty()) {
		// Unconditional branch
		assert(!FBB && "Unconditional branch with multiple successors");
		addDefaultPred(BuildMI(&MBB, dl, get(TMS320C64X::branch_p))
								.addMBB(TBB));
	} else {
		// Insert conditional branch with operands sent to us by
		// analyze branch
		BuildMI(&MBB, dl, get(TMS320C64X::brcond_p)).addMBB(TBB)
			.addImm(Cond[0].getImm()).addReg(Cond[1].getReg());
	}

	return 1;
}

unsigned
TMS320C64XInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
	MachineBasicBlock::iterator I = MBB.end();
	unsigned count;

	count = 0;

	while (I != MBB.begin()) {
		--I;
		if (I->getOpcode() != TMS320C64X::branch_p &&
				I->getOpcode() != TMS320C64X::branch_1 &&
				I->getOpcode() != TMS320C64X::branch_2 &&
				I->getOpcode() != TMS320C64X::brcond_p &&
				I->getOpcode() != TMS320C64X::noop)
			break;

		// Remove branch
		I->eraseFromParent();
		I = MBB.end();
		++count;
	}

	return count;
}

bool
TMS320C64XInstrInfo::isMoveInstr(const MachineInstr &MI, unsigned &src_reg,
				unsigned &dst_reg, unsigned &src_sub_idx,
				unsigned &dst_sub_idx) const
{

	if (MI.getDesc().getOpcode() == TMS320C64X::mv) {
		src_sub_idx = 0;
		dst_sub_idx = 0;
		src_reg = MI.getOperand(1).getReg();
		dst_reg = MI.getOperand(0).getReg();

		// If either of the source/destination registers is on the B
		// side, then this isn't just a simple move instruction, it
		// actually has some functionality, ie 1) actually moving the
		// register from an inaccessable side to the accessable one, and
		// 2) preventing me from stabing my eyes out
		//
		// Unfortunately it's even more complex than that: we want to
		// coalesce moves of B15 so that we can make SP-relative loads
		// and stores as normal; but we don't want to do that for any
		// of the B-side argument registers, lest we end up with
		// operations that write directly to the B side, which then
		// touches the only-want-to-use-side-A situation.
		
		if ((TMS320C64X::BRegsRegClass.contains(src_reg) &&
				src_reg != TMS320C64X::B15) ||
				(TMS320C64X::BRegsRegClass.contains(dst_reg) &&
				dst_reg != TMS320C64X::B15))
				
			return false;

		return true;
	}

	return false;
}
