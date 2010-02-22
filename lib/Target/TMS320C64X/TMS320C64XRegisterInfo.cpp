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
		TMS320C64X::A14, TMS320C64X::B14,
		TMS320C64X::A15, TMS320C64X::B15
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

	llvm_unreachable_internal("Unimplemented function hasFP\n");
}

void
TMS320C64XRegisterInfo::eliminateFrameIndex(
	MachineBasicBlock::iterator I, int SPAdj, RegScavenger *r) const
{

	llvm_unreachable_internal("Unimplemented function eliminateFrameIndex\n");
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
	MFI->setOffsetAdjustment(8); // Return ptr, frame ptr

	// Emit setup instructions
	// Store return pointer - we could use the correct addressing mode
	// to decrement SP for us, but I don't know the infrastructure well
	// enough to do that yet
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::stw))
			.addReg(TMS320C64X::B3).addReg(TMS320C64X::B15);
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::sub_i5), TMS320C64X::B15)
			.addReg(TMS320C64X::B15).addImm(4);
	// Store FP
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::stw))
			.addReg(TMS320C64X::A15).addReg(TMS320C64X::B15);
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::sub_i5), TMS320C64X::B15)
			.addReg(TMS320C64X::B15).addImm(4);
	// Load new FP, adjust SP to account for frame info
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::add_i5), TMS320C64X::A15)
			.addReg(TMS320C64X::B15).addImm(8);
	if (frame_size > 0xFFFF)
		llvm_unreachable("Frame size over 2^16 in emitPrologue");
	// On the assumption the stack size will be sizeable, load
	// constant into volatile register.  XXX - doesn't appear to be a way
	// of generating a constant node from this position
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::mvkl), TMS320C64X::A0)
			.addImm(frame_size);
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::mvkh), TMS320C64X::A0)
			.addImm(frame_size);
	BuildMI(MBB, MBBI, dl, TII.get(TMS320C64X::sub_r), TMS320C64X::B15)
			.addReg(TMS320C64X::B15).addReg(TMS320C64X::A0);
}

void
TMS320C64XRegisterInfo::emitEpilogue(MachineFunction &MF,
					MachineBasicBlock &MBB) const
{

	llvm_unreachable_internal("Unimplemented function emitEpilogue\n");
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
