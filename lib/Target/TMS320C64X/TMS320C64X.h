//===-- TMS320C64X.h - Some generic definitions for TMS320C64X --*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_TMS320C64X_H
#define LLVM_TARGET_TMS320C64X_TMS320C64X_H

// So, uh, yeah, not certain precisely what to put in here...

#include "llvm/Target/TargetMachine.h"

namespace llvm {
	extern Target TheTMS320C64XTarget;
	class FunctionPass;
	FunctionPass *TMS320C64XCreateInstSelector(TargetMachine &TM);
}


#endif //LLVM_TARGET_TMS320C64X_TMS320C64X_H
