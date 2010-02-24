//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

//Some arithmatic and logic

def add_r : TMSInst<(outs GPRegs:$reg), (ins GPRegs:$reg1, GPRegs:$reg2),
	"add $reg1, $reg2, $reg",
	[(set GPRegs:$reg, (add GPRegs:$reg1, GPRegs:$reg2))], UnitL>;

def add_i5 : TMSInst<(outs GPRegs:$reg),
	(ins GPRegs:$reg1, i32imm:$immarg),
	"add $reg1, $immarg, $reg",
	[(set GPRegs:$reg, (add GPRegs:$reg1, sconst5:$immarg))],
	UnitL>;

def sub_r : TMSInst<(outs GPRegs:$reg), (ins GPRegs:$reg1, GPRegs:$reg2),
	"sub $reg1, $reg2, $reg",
	[(set GPRegs:$reg, (sub GPRegs:$reg1, GPRegs:$reg2))], UnitL>;

def sub_i5 : TMSInst<(outs GPRegs:$reg),
	(ins GPRegs:$reg1, i32imm:$immarg),
	"sub $reg1, $immarg, $reg",
	[(set GPRegs:$reg, (sub GPRegs:$reg1, sconst5:$immarg))],
	UnitL>;

// Loading consts: almost always as halfwords
def mvk : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvk $val, $reg",
	[(set GPRegs:$reg, mvk_pred:$val)], UnitS>;

// FIXME: make mvkl and mvkh operate on halfword regs. Which needs hword regs.
def mvkl : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvkl $val, $reg",
	[(set GPRegs:$reg, set_lo:$val)], UnitS>;

def mvkh : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvkh $val, $reg",
	[(set GPRegs:$reg, set_hi:$val)], UnitS>;

def mvkall: TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvkall: fix badgers please",
	[(set GPRegs:$reg, imm:$val)], UnitS>;

def : Pat<(i32 mvk_pred:$val), (mvk imm:$val)>;
def : Pat<(i32 mvk_all_pred:$val), (mvkall imm:$val)>;

// Loads: we can handle all post/pre inc/dec modes, and all indexing situations,
// with a) alignment limitation, b) offset limitation, c) sign extension
def ldb : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldb $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi8 addr:$ptr))], UnitD>;

def ldbu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldbu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi8 addr:$ptr))], UnitD>;

// FIXME: Memory alignment ;_;
def ldh : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldh $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi16 addr:$ptr))], UnitD>;

def ldhu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldhu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi16 addr:$ptr))], UnitD>;

def ldw : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi32 addr:$ptr))], UnitD>;

def stb : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stb $reg, $ptr",
	[(truncstorei8 GPRegs:$reg, addr:$ptr)], UnitD>;

def sth : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"sth $reg, $ptr",
	[(truncstorei16 GPRegs:$reg, addr:$ptr)], UnitD>;

def stw : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, addr:$ptr)], UnitD>;

def stw_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, idxaddr:$ptr)], UnitD>;



// ????

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;
// FIXME: how the hell do we manage spilling of link register?
def ret : TMSInst<(outs), (ins), "br B29 also explode", [(retflag)], UnitS>;
