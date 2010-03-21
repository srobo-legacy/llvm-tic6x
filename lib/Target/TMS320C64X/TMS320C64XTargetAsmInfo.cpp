//===-- TMS320C64XTargetAsmInfo.cpp - TMS320C64X asm properties -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#include "TMS320C64XTargetAsmInfo.h"
using namespace llvm;

TMS320C64XTargetAsmInfo::TMS320C64XTargetAsmInfo(const Target &T,
						const StringRef &TT) {
	Data16bitsDirective = "\t.hword\t";
	Data32bitsDirective = "\t.word\t";
	Data64bitsDirective = "\t.dword\t";
	ZeroDirective = "\t.zero\t";
	CommentString = ";";

	AlignmentIsInBytes = false;
}
