//===-- SparcTargetInfo.cpp - Sparc Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from Sparc implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;

Target llvm::TheTMS320C64XTarget;

extern "C" void LLVMInitializeTMS320C64XTargetInfo() { 
  RegisterTarget<Triple::tms320c64x> X(TheTMS320C64XTarget, "tms320c64x", "TMS320C64X");
}
