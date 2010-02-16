//===-- TMS320C64XGenCallingConv.td - TMS320C64X Calling convention  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

def RetCC_TMS320C64X : CallingConv<[
	// Return values -> A12
	CCIfType<[i32], CCAssignToReg<[A12]>>
]>;

def CC_TMS320C64X : CallingConv<[
	// I'm not aware of any existing CC for TIs DSPs, but there probably
	// is one. Until I find it, instead pass arguments in set of middle
	// registers (out of volatile range, condition range), spill rest
	// to stack.
	CCIfType<[i32], CCAssignToReg<[A12,A13,A14,A15,B12,B13,B14,B15]>>,
	CCAssignToStack<4, 4>
]>;
