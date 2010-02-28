//===-- TMS320C64XTargetMachine.cpp - Define TargetMachine for TMS320C64X --==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "TMS320C64XTargetMachine.h"
#include "TMS320C64XTargetAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Target/TargetAsmInfo.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeTMS320C64XTarget() {
	// Register the target.
	RegisterTargetMachine<TMS320C64XTargetMachine> X(TheTMS320C64XTarget);
	RegisterAsmInfo<TMS320C64XTargetAsmInfo> Z(TheTMS320C64XTarget);
}

TMS320C64XTargetMachine::TMS320C64XTargetMachine(const Target &T,
						const std::string &TT,
						const std::string &FS) :
	LLVMTargetMachine(T, TT),
	Subtarget(),
	DataLayout("e-p:32:32:32-i8:8:8-i16:16:16-i32:32:32-n32"),
	/* No float types - could define n40, in that the DSP supports 40 bit
	 * arithmatic, however it doesn't support it for all logic operations,
	 * only a variety of alu ops. */
	InstrInfo(*this), TLInfo(*this),
	FrameInfo(TargetFrameInfo::StackGrowsDown, 4, -8)
{
}


bool TMS320C64XTargetMachine::addInstSelector(PassManagerBase &PM,
						CodeGenOpt::Level OptLevel) {
	PM.add(TMS320C64XCreateInstSelector(*this));
	return false;
}

