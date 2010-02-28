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

	if (dst_class != src_class)
		llvm_unreachable("copyRegToReg sees nonexistant registerclass");

	DebugLoc DL = DebugLoc::getUnknownLoc();
	BuildMI(MBB, I, DL, get(TMS320C64X::add_i5), dst_reg).addReg(src_reg).addImm(0);
	return true;
}

void
TMS320C64XInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator I, unsigned src_reg,
		bool is_kill, int FI, const TargetRegisterClass *rc) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();

	if (rc != TMS320C64X::GPRegsRegisterClass)
		llvm_unreachable("Unknown register class in spillslot");

	BuildMI(MBB, I, DL, get(TMS320C64X::stw_idx)).addReg(TMS320C64X::A15)
			.addFrameIndex(FI) .addReg(src_reg);
}

void
TMS320C64XInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator MI, unsigned dst_reg, int frame_idx,
		const TargetRegisterClass *rc) const
{
	DebugLoc DL = DebugLoc::getUnknownLoc();

	if (rc != TMS320C64X::GPRegsRegisterClass)
		llvm_unreachable("Unknown register class in loadslot");

	BuildMI(MBB, MI, DL, get(TMS320C64X::ldw_idx)).addReg(TMS320C64X::A15)
			.addFrameIndex(frame_idx).addReg(dst_reg);
}
