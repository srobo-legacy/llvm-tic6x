//==- TMS320C64X.td - TMS320C64X Target Machine definitions -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
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
  let isLittleEndianEncoding = 1;
}

def TMS320C64X : Target {
  let InstructionSet = TMS320C64XInstrInfo;
}
