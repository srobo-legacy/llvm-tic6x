//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

// misc, to begin with

def SDT_Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_call_seq_start : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_call_seq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_call_seq_start,
				[SDNPHasChain, SDNPOutFlag]>;
def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_call_seq_end,
				[SDNPHasChain, SDNPOptInFlag, SDNPOutFlag]>;

def fake_call_start : TMSInst<(outs), (ins i32imm:$val), "#callstart",
				[(callseq_start timm:$val)], UnitD>;
def fake_call_end : TMSInst<(outs), (ins i32imm:$val, i32imm:$val2), "#callend",
				[(callseq_end timm:$val, timm:$val2)], UnitD>;

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;
// FIXME: how the hell do we manage spilling of link register?
def ret : TMSInst<(outs), (ins), "br B29 also explode", [(retflag)], UnitS>;




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
def : Pat<(i32 (Wrapper tglobaladdr:$dst)), (mvkall tglobaladdr:$dst)>;

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



// Branch / call instructions

def callp : TMSInst<(outs), (ins calltarget:$dst),
			"callp $dst", [(TMS320C64Xcall tglobaladdr:$dst)],
			UnitS> {
	let isCall = 1;
	let Defs = [A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A16, A17, A18, A19,
		A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
		B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B16, B17, B18, B19,
		B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31];
}
