//====-- TMS320C64XTargetAsmInfo.h - TMS320C64X asm properties -*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_ASMINFO_H
#define LLVM_TARGET_TMS320C64X_ASMINFO_H

#include "llvm/Target/TargetAsmInfo.h"

namespace llvm {
	class Target;
	class StringRef;
	struct TMS320C64XTargetAsmInfo : public TargetAsmInfo {
	explicit TMS320C64XTargetAsmInfo(const Target &T, const StringRef &TT);
};
} // namespace llvm

#endif
