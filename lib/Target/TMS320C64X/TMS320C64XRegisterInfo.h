//===- MSP430RegisterInfo.h - MSP430 Register Information Impl --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_REGISTERINFO_H
#define LLVM_TARGET_TMS320C64X_REGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"
// Include here: register definition table

namespace llvm {

class TargetInstrInfo;
class TMS320C64XTargetMachine;

// FIXME: make register info extend table definition
struct TMS320C64XRegisterInfo : TargetRegisterInfo /*: public TMS320C64XGenRegisterInfo*/ {
private:
	TMS320C64XTargetMachine &TM;
	const TargetInstrInfo &TII;

public:
	TMS320C64XRegisterInfo(TMS320C64XTargetMachine &tm,
				const TargetInstrInfo &tii);

	const unsigned int *getCalleeSavedRegs(const MachineFunction *) const;
	const TargetRegisterClass* const*
		getCalleeSavedRegClasses(const MachineFunction *) const;
	BitVector getReservedRegs(const MachineFunction &MF) const;
	unsigned int getSubReg(unsigned int, unsigned int) const;
	bool hasFP(const MachineFunction &MF);
	void eliminateFrameIndex(MachineBasicBlock::iterator I, int SPAdj
					RegScavenger *r) const;
	void emitPrologue(MachineFunction &MF) const;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlok &MBB) const;

	/* Debug stuff, apparently */
	unsigned int getRARegister() const;
	unsigned int getFrameRegister(MachineFunction &MF) const;
	int getDwarfRegNum(unsigned RegNum, bool isEH) const;
};

} // llvm namespace

#endif // LLVM_TARGET_TMS320C64X_REGISTERINFO_H
