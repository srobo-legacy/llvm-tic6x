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

struct TMS320C64XRegisterInfo : public TMS320C64XGenRegisterInfo {
private:
	TMS320C64XTargetMachine &TM;
	const TargetInstrInfo &TII;

public:
	TMS320C64XRegisterInfo(TMS320C64XTargetMachine &tm,
				const TargetInstrInfo &tii);

	/* Yet again, avoid implementing anything machdep until it's needed */
};

} // llvm namespace

#endif // LLVM_TARGET_TMS320C64X_REGISTERINFO_H
