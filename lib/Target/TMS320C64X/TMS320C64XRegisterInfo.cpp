//===- TMS320C64XRegisterInfo.cpp - TMS320C64X Register Information -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XTargetMachine.h"

// Actual register information
#include "TMS320C64XGenRegisterInfo.inc"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

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
		TMS320C64X::A10, TMS320C64X::B10,
		TMS320C64X::A11, TMS320C64X::B11,
		TMS320C64X::A12, TMS320C64X::B12,
		TMS320C64X::A13, TMS320C64X::B13,
		TMS320C64X::B14,
	};

	return nonvolatileRegs;
}

const TargetRegisterClass* const*
TMS320C64XRegisterInfo::getCalleeSavedRegClasses(MachineFunction const*) const
{
	static const TargetRegisterClass *const calleeNonvolatileRegClasses[] ={
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass,
		&TMS320C64X::GPRegsRegClass, &TMS320C64X::GPRegsRegClass
	};

	return calleeNonvolatileRegClasses;
}

unsigned int
TMS320C64XRegisterInfo::getSubReg(unsigned int, unsigned int) const
{

	llvm_unreachable_internal("Unimplemented function getSubReg\n");
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

void
TMS320C64XRegisterInfo::eliminateFrameIndex(
	MachineBasicBlock::iterator I, int SPAdj, RegScavenger *r) const
{
	unsigned i, frame_index, offs;

	MachineInstr &MI = *I;
	MachineFunction &MF = *MI.getParent()->getParent();
	i = 0;

	while (!MI.getOperand(i).isFI())
		++i;

	assert(i < MI.getNumOperands() && "No FrameIndex in eliminateFrameIdx");
	frame_index = MI.getOperand(i).getIndex();
	offs = MF.getFrameInfo()->getObjectOffset(frame_index);

	MI.getOperand(i).ChangeToImmediate(offs);
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

	frame_size = MFI->getStackSize();
	frame_size += 8;

	// Emit setup instructions
	// Store return pointer - we could use the correct addressing mode
	// to decrement SP for us, but I don't know the infrastructure well
	// enough to do that yet
	addDefaultPred(BuildMI(MBB, MBBI, dl,
		TII.get(TMS320C64X::word_idx_store2))
		.addReg(TMS320C64X::B15).addImm(0).addReg(TMS320C64X::B3));

	// Store FP
	addDefaultPred(BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::mvk_p))
		.addReg(TMS320C64X::A0, RegState::Define).addImm(-4));
	addDefaultPred(BuildMI(MBB, MBBI, dl,
		TII.get(TMS320C64X::word_idx_store2))
		.addReg(TMS320C64X::B15).addReg(TMS320C64X::A0, RegState::Kill)
		.addReg(TMS320C64X::A15));

	// Setup our own FP using the current SP
	addDefaultPred(BuildMI(MBB, MBBI, dl,
		TII.get(TMS320C64X::mv))
		.addReg(TMS320C64X::A15).addReg(TMS320C64X::B15));

	// On the assumption the stack size will be sizeable, load
	// constant into volatile register.  XXX - doesn't appear to be a way
	// of generating a constant node from this position
	if (frame_size < 0x8000) {
		addDefaultPred(BuildMI(MBB, MBBI, dl,TII.get(TMS320C64X::mvk_p))
			.addReg(TMS320C64X::A0, RegState::Define)
			.addImm(frame_size));
	} else {
		addDefaultPred(BuildMI(MBB, MBBI, dl,
			TII.get(TMS320C64X::mvkl_p))
			.addReg(TMS320C64X::A0, RegState::Define)
			.addImm(frame_size));
		addDefaultPred(BuildMI(MBB, MBBI, dl,
			TII.get(TMS320C64X::mvkh_p))
			.addReg(TMS320C64X::A0)
			.addImm(frame_size).addReg(TMS320C64X::A0));
	}

	addDefaultPred(BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::sub_p_rr),
		TMS320C64X::B15).addReg(TMS320C64X::B15)
		.addReg(TMS320C64X::A0, RegState::Kill));
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

	// To finish, nuke stack frame, restore FP, ret addr

	addDefaultPred(BuildMI(MBB, MBBI, DL,
		TII.get(TMS320C64X::mv))
		.addReg(TMS320C64X::B15).addReg(TMS320C64X::A15));
	addDefaultPred(BuildMI(MBB, MBBI, DL, TII.get(TMS320C64X::mvk_p))
		.addReg(TMS320C64X::A0, RegState::Define).addImm(-4));
	addDefaultPred(BuildMI(MBB, MBBI, DL,
		TII.get(TMS320C64X::word_idx_load2))
		.addReg(TMS320C64X::A15, RegState::Define)
		.addReg(TMS320C64X::B15)
		.addReg(TMS320C64X::A0, RegState::Kill));
	addDefaultPred(BuildMI(MBB, MBBI, DL,
		TII.get(TMS320C64X::word_idx_load2))
		.addReg(TMS320C64X::B3, RegState::Define)
		.addReg(TMS320C64X::B15).addImm(0));
}

int
TMS320C64XRegisterInfo::getDwarfRegNum(unsigned reg_num, bool isEH) const
{

	llvm_unreachable_internal("Unimplemented function getDwarfRegNum\n");
}

unsigned int
TMS320C64XRegisterInfo::getRARegister() const
{

	llvm_unreachable_internal("Unimplemented function getRARegister\n");
}

unsigned int
TMS320C64XRegisterInfo::getFrameRegister(MachineFunction &MF) const
{

	llvm_unreachable_internal("Unimplemented function getFrameRegister\n");
}
