//===- TMS320C64XRegisterInfo.cpp - TMS320C64X Register Information -------===//
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

#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XTargetMachine.h"

// Actual register information
#include "TMS320C64XGenRegisterInfo.inc"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

TMS320C64XRegisterInfo::TMS320C64XRegisterInfo(TMS320C64XTargetMachine &tm,
						const TargetInstrInfo &tii)
  : TMS320C64XGenRegisterInfo(),
    TM(tm), TII(tii)
{

	return;
}

TMS320C64XRegisterInfo::~TMS320C64XRegisterInfo()
{

	/* as ever, nothing */
	return;
}

BitVector
TMS320C64XRegisterInfo::getReservedRegs(const MachineFunction &MF) const
{

	BitVector Reserved(getNumRegs());
	Reserved.set(TMS320C64X::B15);
	Reserved.set(TMS320C64X::A15);
	Reserved.set(TMS320C64X::A14);
	return Reserved;
}

const unsigned *
TMS320C64XRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const
{
	static const unsigned nonvolatileRegs[] = {
		TMS320C64X::A10,
		TMS320C64X::A11,
		TMS320C64X::A12,
		TMS320C64X::A13,
		0
	};

	return nonvolatileRegs;
}

const TargetRegisterClass* const*
TMS320C64XRegisterInfo::getCalleeSavedRegClasses(MachineFunction const*) const
{
	static const TargetRegisterClass *const calleeNonvolatileRegClasses[] ={
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass
	};

	return calleeNonvolatileRegClasses;
}

unsigned int
TMS320C64XRegisterInfo::getSubReg(unsigned int, unsigned int) const
{

	llvm_unreachable("Unimplemented function getSubReg\n");
}

bool
TMS320C64XRegisterInfo::hasFP(const MachineFunction &MF) const
{

	// Guidelines say that we should only return true if the function
	// has any variable sized arrays that get allocated on the stack,
	// so that anything else can be calculated relative to the stack
	// pointer. This is all fine, and would optimised a lot of things
	// seeing how then we wouldn't need to load stack offsets to a register
	// each time (they'd be positive).
	// However this means extra work and testing, so it's room for expansion
	// and optimisation in the future.
	return true;
}

bool
TMS320C64XRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const
{

	return true;
}

unsigned
TMS320C64XRegisterInfo::eliminateFrameIndex(
	MachineBasicBlock::iterator MBBI, int SPAdj, int *Vaule,
	RegScavenger *r) const
{
	unsigned i, frame_index, reg, access_alignment;
	int offs;

	/* XXX - Value turned up in 2.7, I don't know what it does. */

	MachineInstr &MI = *MBBI;
	MachineFunction &MF = *MI.getParent()->getParent();
	MachineBasicBlock &MBB = *MI.getParent();
	DebugLoc dl = DebugLoc::getUnknownLoc();
	i = 0;

	while (!MI.getOperand(i).isFI())
		++i;

	assert(i < MI.getNumOperands() && "No FrameIndex in eliminateFrameIdx");
	frame_index = MI.getOperand(i).getIndex();
	offs = MF.getFrameInfo()->getObjectOffset(frame_index);

	const TargetInstrDesc tid = MI.getDesc();
	access_alignment = (tid.TSFlags & TMS320C64XII::mem_align_amt_mask)
				>> TMS320C64XII::mem_align_amt_shift;

	// Firstly, is this actually memory access? Might be lea (vomit)
	if (!(MI.getDesc().TSFlags & TMS320C64XII::is_memaccess)) {
		// If so, the candidates are sub and add - each of which
		// have an sconst5 range. If the offset doesn't fit in there,
		// need to scavenge a register
		if (check_sconst_fits(offs, 5)) {
			MI.getOperand(i).ChangeToImmediate(offs);
			return 0;
		}
		access_alignment = 0;
	// So for memory, will this frame index actually fit inside the
	// instruction field?
	} else if (check_uconst_fits(abs(offs), 5 + access_alignment)) {
		// We can just punt this constant into the instruction and
		// it'll be scaled appropriately

		MI.getOperand(i).ChangeToImmediate(offs);
		return 0;
	}

	// Otherwise, we need to do some juggling to load that constant into
	// a register correctly. First of all, because of the highly-unpleasent
	// scaling feature of using indexing instructions we need to shift
	// the stack offset :|
	if (offs & ((1 << access_alignment) -1 ))
		llvm_unreachable("Unaligned stack access - should never occur");

	offs >>= access_alignment;

	const TargetRegisterClass *c;
	if (tid.TSFlags & TMS320C64XII::unit_2)
		c = TMS320C64X::BRegsRegisterClass;
	else
		c = TMS320C64X::ARegsRegisterClass;

	reg = r->FindUnusedReg(c);

	if (reg == 0) {
		// XXX - this kicks a register out and lets us use it but...
		// that'll lead to a store, to a stack slot, which will mean
		// this method is called again. Explosions?
		reg = r->scavengeRegister(c, MBBI, 0);
	}

	// XXX - this will explode when the stack offset is > 2^15, at which
	// point we need to start pairing mvkl/mvkh. Don't worry about that for
	// now, because the assembler will start complaining when that occurs
	addDefaultPred(BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::mvk_p))
		.addReg(reg, RegState::Define).addImm(offs));

	MI.getOperand(i).ChangeToRegister(reg, false, false, true);
	return 0;
}

void
TMS320C64XRegisterInfo::emitPrologue(MachineFunction &MF) const
{
	int frame_size;

	MachineBasicBlock &MBB = MF.front();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	MachineBasicBlock::iterator MBBI = MBB.begin();
	DebugLoc dl = (MBBI != MBB.end() ? MBBI->getDebugLoc()
				: DebugLoc::getUnknownLoc());

	// Mark return address as being a live in - don't mark it as such for
	// the whole function, because we want to save it manually. Otherwise
	// extra code will be generated to store it elsewhere.
	// Ideally we don't need to save manually, but I call this easier
	// to debug.
	MBB.addLiveIn(TMS320C64X::B3);
	frame_size = MFI->getStackSize();
	frame_size += 8;

	// Align the size of the stack - has to remain double word aligned.
	frame_size += 7;
	frame_size &= ~7;

	// Emit setup instructions - unfortunately because they have to be
	// done in parallel now, this can't currently be modeled through llvm,
	// so instead we hack this in at the assembly printing stage.
	addDefaultPred(BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::prolog))
		.addImm(frame_size));
}

void
TMS320C64XRegisterInfo::emitEpilogue(MachineFunction &MF,
					MachineBasicBlock &MBB) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();
	const MachineFrameInfo *MFI = MF.getFrameInfo();
	MachineBasicBlock::iterator MBBI = prior(MBB.end());

	if (MFI->hasVarSizedObjects())
		llvm_unreachable("Can't currently support varsize stack frame");

	if (MBBI->getOpcode() != TMS320C64X::ret)
		llvm_unreachable("Can't insert epilogue before non-ret insn");

	// For current situation, epilog has to be hard coded to allow
	// parallel instructions to work, hence this unpleasent hack.
	addDefaultPred(BuildMI(MBB, MBBI, DL, TII.get(TMS320C64X::epilog)));
}

int
TMS320C64XRegisterInfo::getDwarfRegNum(unsigned reg_num, bool isEH) const
{

	llvm_unreachable("Unimplemented function getDwarfRegNum\n");
}

unsigned int
TMS320C64XRegisterInfo::getRARegister() const
{

	llvm_unreachable("Unimplemented function getRARegister\n");
}

unsigned int
TMS320C64XRegisterInfo::getFrameRegister(const MachineFunction &MF) const
{

	llvm_unreachable("Unimplemented function getFrameRegister\n");
}
