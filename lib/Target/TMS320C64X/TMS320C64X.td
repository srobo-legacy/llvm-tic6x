//==- TMS320C64X.td - TMS320C64X Target Machine definitions -*- tablegen -*-==//
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

// Get the target-independent interfaces which we are implementing.
//
include "llvm/Target/Target.td"

class CodeFrag<dag frag> {
  dag Fragment = frag;
}

include "TMS320C64XRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Instruction formats, instructions
//===----------------------------------------------------------------------===//

include "TMS320C64XInstrFormats.td"
include "TMS320C64XInstrInfo.td"

//===----------------------------------------------------------------------===//
// Subtarget features:
//===----------------------------------------------------------------------===//

// N/A

//===----------------------------------------------------------------------===//
// Calling convention:
//===----------------------------------------------------------------------===//

include "TMS320C64XGenCallingConv.td"

// Target:

def TMS320C64XInstrInfo : InstrInfo {
  let TSFlagsFields = ["unit_bits", "unit_no", "delay_slots", "mem_access", "mem_load_store", "mem_shift_amt"];
  let TSFlagsShifts = [0, 2, 3, 6, 7, 8];
  let isLittleEndianEncoding = 1;
}

def TMS320C64X : Target {
  let InstructionSet = TMS320C64XInstrInfo;
}
