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
	// XXX - for 64 bit returns I assume we use the A4/A5 pair.. uh..
	// need to actually check this in spec though
	CCIfType<[i32, f32], CCAssignToReg<[A4,A5]>>
]>;

def CC_TMS320C64X : CallingConv<[
	// XXX - dword arguments should go in register pairs. Nonexistant
	// right now.
	CCIfType<[i8,i16], CCPromoteToType<i32>>,
	CCIfType<[i32,f32], CCAssignToReg<[A4,B4,A6,B6,A8,B8,A10,B10,A12,B12]>>,
	CCAssignToStack<0, 0>
]>;
