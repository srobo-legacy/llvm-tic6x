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

const unsigned int*
TMS320C64XRegisterInfo::getCalleeSavedRegs(MachineFunction const*) const
{

	llvm_unreachable_internal("Unimplemented function getCalleeSavedRegs\n");
}

const TargetRegisterClass* const*
TMS320C64XRegisterInfo::getCalleeSavedRegClasses(MachineFunction const*) const
{

	llvm_unreachable_internal("Unimplemented function getCalleeSavedRegClasses\n");
}

BitVector
TMS320C64XRegisterInfo::getReservedRegs(const MachineFunction &MF) const
{

	llvm_unreachable_internal("Unimplemented function getReservedRegs\n");
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

	llvm_unreachable_internal("Unimplemented function emitPrologue\n");
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
