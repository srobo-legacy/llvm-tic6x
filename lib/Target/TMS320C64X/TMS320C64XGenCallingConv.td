//===-- TMS320C64XGenCallingConv.td - TMS320C64X Calling convention  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

def RetCC_TMS320C64X : CallingConv<[
	// XXX - returning structs is a black arg
	CCIfType<[i8,i16], CCPromoteToType<i32>>,
	CCIfType<[i32], CCAssignToReg<[A4]>>
]>;

def CC_TMS320C64X : CallingConv<[
	// XXX - dword arguments should go in register pairs. Nonexistant
	// right now.
	CCIfType<[i8,i16], CCPromoteToType<i32>>,
	CCIfType<[i32], CCAssignToReg<[A4,B4,A6,B6,A8,B8,A10,B10,A12,B12]>>,
	CCAssignToStack<4, 4>
]>;
