//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

// Loading consts: almost always as halfwords
def mvkl : TMSInst<(outs GPRegs:$reg), (ins int_operand:$val), "mvkl",
		[(set GPRegs:$reg, set_lo:$val)], UnitS>;
def mvkh : TMSInst<(outs GPRegs:$reg), (ins int_operand:$val), "mvkh",
		[(set GPRegs:$reg, set_hi:$val)], UnitS>;

def mvkall: TMSInst<(outs GPRegs:$reg), (ins i32imm:$val), "mvkall",
		[(set GPRegs:$reg, imm:$val)], UnitS>;

// Loads: we can handle all post/pre inc/dec modes, and all indexing situations,
// with a) alignment limitation, b) offset limitation, c) sign extension
def ldb : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr), "ldb",
	[(set GPRegs:$dst, (sextloadi8 addr:$ptr))], UnitD>;

def ldbu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr), "ldbu",
	[(set GPRegs:$dst, (zextloadi8 addr:$ptr))], UnitD>;

// FIXME: Memory alignment ;_;
def ldh : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr), "ldh",
	[(set GPRegs:$dst, (sextloadi16 addr:$ptr))], UnitD>;

def ldhu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr), "ldhu",
	[(set GPRegs:$dst, (zextloadi16 addr:$ptr))], UnitD>;

def ldw : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr), "ldw",
	[(set GPRegs:$dst, (sextloadi32 addr:$ptr))], UnitD>;

def stb : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg), "stb",
	[(truncstorei8 GPRegs:$reg, addr:$ptr)], UnitD>;

def sth : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg), "sth",
	[(truncstorei16 GPRegs:$reg, addr:$ptr)], UnitD>;

def stw : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg), "stw",
	[(store GPRegs:$reg, addr:$ptr)], UnitD>;

def stw_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg), "stw",
			[(store GPRegs:$reg, idxaddr:$ptr)], UnitD>;



// ????

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;
// FIXME: how the hell do we manage spilling of link register?
def ret : TMSInst<(outs), (ins), "br B29", [(retflag)], UnitS>;
