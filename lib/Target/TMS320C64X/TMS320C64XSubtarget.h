//==-- TMS320C64XSubtarget.h - Define Subtarget for TMS320C64X --*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_SUBTARGET_H
#define LLVM_TARGET_TMS320C64X_SUBTARGET_H

#include "llvm/Target/TargetSubtarget.h"

#include <string>

namespace llvm {

class TMS320C64XSubtarget : public TargetSubtarget {
public:
	TMS320C64XSubtarget();
};

} // llvm namespace

#endif  // LLVM_TARGET_TMS320C64X_SUBTARGET_H
