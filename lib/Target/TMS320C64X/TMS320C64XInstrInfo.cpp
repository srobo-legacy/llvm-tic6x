//===- TMS320C64XInstrInfo.cpp - TMS320C64X Instruction Information -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
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

	addDefaultPred(BuildMI(MBB, I, DL, get(TMS320C64X::word_idx_store_p))
		.addReg(TMS320C64X::A15).addFrameIndex(FI)
		.addReg(src_reg, getKillRegState(is_kill)));
}

void
TMS320C64XInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator MI, unsigned dst_reg, int frame_idx,
		const TargetRegisterClass *rc) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();

	addDefaultPred(BuildMI(MBB, MI, DL, get(TMS320C64X::word_idx_load_p))
		.addReg(dst_reg, RegState::Define)
		.addReg(TMS320C64X::A15).addFrameIndex(frame_idx));
}

bool
TMS320C64XInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
		MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
		SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const
{
	bool predicated, found_cond_branch, saw_uncond_branch;
	int pred_idx, opcode;
	MachineBasicBlock::iterator I = MBB.end();

	found_cond_branch = false;
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

			// If there was already a conditional branch, freak out
			if (found_cond_branch) {
				TBB = NULL;
				FBB = NULL;
				return 1;
			}

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
		if (predicated && (opcode == TMS320C64X::brcond_p ||
					opcode == TMS320C64X::brcond_1 ||
					opcode == TMS320C64X::brcond_2)) {
			// Two different conditions to consider - this is the
			// only branch, in which case we fall through to the
			// next, or it's a conditional before unconditional.

			if (TBB != NULL) { // unconditional was seen
				FBB = I->getOperand(0).getMBB();
			} else {
				TBB = I->getOperand(0).getMBB();
			}

			// Only other thing we need to do is work out the
			// conditional situation - go looking for condition
			// instructions.
			found_cond_branch = true;
			continue;	
		}

		if (found_cond_branch) {
			// Look for condition instruction... there are a _lot_
			// of them, so we shall revert to something unpleasent
			if (!strncmp(I->getDesc().getName(), "cmp", 3)) {
				// It appears the stuff we put in the Cond
				// list is pretty ad-hoc and machdep - so
				// I'm just going to bung the instruction
				// opcode and its operands in

				Cond.push_back(
					MachineOperand::CreateImm(opcode));
				// FIXME - need to duplicate operands?
				Cond.push_back(I->getOperand(1));
				Cond.push_back(I->getOperand(2));
				found_cond_branch = false;
				break;
			} else {
				continue;
			}
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

	if (found_cond_branch)
		// We must have got lost looking for it
		return true;

	return false;
}

unsigned
TMS320C64XInstrInfo::InsertBranch(MachineBasicBlock &MBB,
		MachineBasicBlock *TBB, MachineBasicBlock *FBB,
		const SmallVectorImpl<MachineOperand> &Cond) const
{
	DebugLoc dl = DebugLoc::getUnknownLoc();

	assert(TBB && "InsertBranch can't insert fallthroughs");
	assert((Cond.size() == 3 || Cond.size() == 0) &&
			"Invalid condition to InsertBranch");

	if (Cond.empty()) {
		// Unconditional branch
		assert(!FBB && "Unconditional branch with multiple successors");
		addDefaultPred(BuildMI(&MBB, dl, get(TMS320C64X::branch_p))
								.addMBB(TBB));
		return 1;
	} else {
		llvm_unreachable("Can't insert conditional branches yet "
				"on tms320c64x");
	}
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
				I->getOpcode() != TMS320C64X::brcond_1 &&
				I->getOpcode() != TMS320C64X::brcond_2 &&
				I->getOpcode() != TMS320C64X::noop)
			break;

		// Remove branch
		I->eraseFromParent();
		I = MBB.end();
		++count;
	}

	return count;
}
