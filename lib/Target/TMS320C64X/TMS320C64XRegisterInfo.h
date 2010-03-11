//==- TMS320C64XRegisterInfo.h - TMS320C64X Register Information -*- C++ -*-==//
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

#include "TMS320C64XGenRegisterInfo.h.inc"
#include "TMS320C64XGenRegisterNames.inc"

#include "llvm/Target/TargetRegisterInfo.h"

namespace llvm {

class TargetInstrInfo;
class TMS320C64XTargetMachine;

struct TMS320C64XRegisterInfo : TMS320C64XGenRegisterInfo {
private:
	TMS320C64XTargetMachine &TM;
	const TargetInstrInfo &TII;

public:
	TMS320C64XRegisterInfo(TMS320C64XTargetMachine &tm,
				const TargetInstrInfo &tii);
	~TMS320C64XRegisterInfo();

	const unsigned int *getCalleeSavedRegs(const MachineFunction *) const;
	const TargetRegisterClass* const*
		getCalleeSavedRegClasses(const MachineFunction *) const;
	BitVector getReservedRegs(const MachineFunction &MF) const;
	unsigned int getSubReg(unsigned int, unsigned int) const;
	bool hasFP(const MachineFunction &MF) const;
	bool requiresRegisterScavenging(const MachineFunction &MF) const;
	void eliminateFrameIndex(MachineBasicBlock::iterator I, int SPAdj,
					RegScavenger *r) const;
	void emitPrologue(MachineFunction &MF) const;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

	/* Debug stuff, apparently */
	unsigned int getRARegister() const;
	unsigned int getFrameRegister(MachineFunction &MF) const;
	int getDwarfRegNum(unsigned RegNum, bool isEH) const;
};

} // llvm namespace

#endif // LLVM_TARGET_TMS320C64X_REGISTERINFO_H
