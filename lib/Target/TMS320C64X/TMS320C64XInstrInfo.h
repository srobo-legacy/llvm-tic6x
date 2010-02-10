//==- TMS320C64XInstrInfo.h - TMS320C64X Instruction Information -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_INSTRINFO_H
#define LLVM_TARGET_TMS320C64X_INSTRINFO_H

#include "llvm/Target/TargetInstrInfo.h"

namespace llvm {

class TMS320C64XTargetMachine;

class TMX320C64XInstrInfo : public TargetInstrInfoImpl {
	const TMS320C64XRegisterInfo RI;
	TMS320C64XTargetMachine &TM;
public:
	explicit TMS320C64XInstrInfo(TMS320C64XTargetMachine &TM);

	virtual const TargetRegisterInfo &getRegisterInfo() const { return RI; }

	/* MSP430 code that this is based on has a large amount of fudge here
	 * to implement branches, stack slot spills and the like; for the moment
	 * leave unimplemented and fix it when something explodes */
};

} // llvm

#endif // LLVM_TARGET_TMS320C64X_INSTRINFO_H

