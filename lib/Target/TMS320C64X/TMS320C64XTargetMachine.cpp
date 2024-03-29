//===-- TMS320C64XTargetMachine.cpp - Define TargetMachine for TMS320C64X --==//
//
// Copyright 2010 Jeremy Morse <jeremy.morse@gmail.com>. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JEREMY MORSE ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL JEREMY MORSE OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "TMS320C64XTargetMachine.h"
#include "TMS320C64XMCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeTMS320C64XTarget() {
	// Register the target.
	RegisterTargetMachine<TMS320C64XTargetMachine> X(TheTMS320C64XTarget);
	RegisterAsmInfo<TMS320C64XMCAsmInfo> Z(TheTMS320C64XTarget);
}

TMS320C64XTargetMachine::TMS320C64XTargetMachine(const Target &T,
						const std::string &TT,
						const std::string &FS) :
	LLVMTargetMachine(T, TT),
	Subtarget(),
	DataLayout("e-p:32:32:32-i8:8:8-i16:16:16-i32:32:32-f32:32:32-f64:64:64-n32"),
	/* No float types - could define n40, in that the DSP supports 40 bit
	 * arithmatic, however it doesn't support it for all logic operations,
	 * only a variety of alu ops. */
	InstrInfo(*this), TLInfo(*this),
	FrameInfo(TargetFrameInfo::StackGrowsDown, 8, -4)
{
}


bool TMS320C64XTargetMachine::addInstSelector(PassManagerBase &PM,
						CodeGenOpt::Level OptLevel) {
	PM.add(TMS320C64XCreateInstSelector(*this));
	return false;
}

bool TMS320C64XTargetMachine::addPreEmitPass(PassManagerBase &PM,
						CodeGenOpt::Level OptLevel) {

	PM.add(createTMS320C64XDelaySlotFillerPass(*this));
	return true;
}
