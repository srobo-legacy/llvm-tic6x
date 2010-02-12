//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

// Loads: we can handle all post/pre inc/dec modes, and all indexing situations,
// with a) alignment limitation, b) offset limitation, c) sign extension
def ldb : TMSInst<(outs GPRegs:$dst), (ins i8mem:$ptr), "ldb",
	[(set GPRegs:$dst, (sextloadi8 AddrType:$ptr))], UnitD>;

def ldbu : TMSInst<(outs GPRegs:$dst), (ins i8mem:$ptr), "ldbu",
	[(set GPRegs:$dst, (zextloadi8 AddrType:$ptr))], UnitD>;

// FIXME: Memory alignment ;_;
def ldh : TMSInst<(outs GPRegs:$dst), (ins i16mem:$ptr), "ldh",
	[(set GPRegs:$dst, (sextloadi16 AddrType:$ptr))], UnitD>;

def ldhu : TMSInst<(outs GPRegs:$dst), (ins i16mem:$ptr), "ldhu",
	[(set GPRegs:$dst, (zextloadi16 AddrType:$ptr))], UnitD>;

def ldw : TMSInst<(outs GPRegs:$dst), (ins i32mem:$ptr), "ldw",
	[(set GPRegs:$dst, (sextloadi32 AddrType:$ptr))], UnitD>;

def stb : TMSInst<(outs), (ins i8mem:$ptr, GPRegs:$reg), "stb",
	[(truncstorei8 GPRegs:$reg, AddrType:$ptr)], UnitD>;

def sth : TMSInst<(outs), (ins i16mem:$ptr, GPRegs:$reg), "sth",
	[(truncstorei16 GPRegs:$reg, AddrType:$ptr)], UnitD>;

def stw : TMSInst<(outs), (ins i32mem:$ptr, GPRegs:$reg), "stw",
	[(truncstorei32 GPRegs:$reg, AddrType:$ptr)], UnitD>;