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
def SDT_brcond : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>]>;
def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_call_seq_start : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_call_seq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_cmp : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisSameAs<0, 1>]>;

// This just copies the last two operands to the node being operated upon into
// the output operands: this makes tablegen happy. Note that this will probably
// explode if ever applied to an automagically generated node, should only be
// used for custom-generated-nodes
def predwrapper : ComplexPattern<OtherVT, 2, "bounce_predicate", [], []>;

def brcond_node : SDNode<"TMSISD::BRCOND", SDT_brcond, [SDNPHasChain]>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_call_seq_start,
				[SDNPHasChain, SDNPOutFlag]>;
def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_call_seq_end,
				[SDNPHasChain, SDNPOptInFlag, SDNPOutFlag]>;
def cmpeq_node : SDNode<"TMSISD::CMPEQ", SDT_cmp, [SDNPHasChain]>;
def cmpgt_node : SDNode<"TMSISD::CMPGT", SDT_cmp, [SDNPHasChain]>;
def cmpgtu_node : SDNode<"TMSISD::CMPGTU", SDT_cmp, [SDNPHasChain]>;
def cmplt_node : SDNode<"TMSISD::CMPLT", SDT_cmp, [SDNPHasChain]>;
def cmpltu_node : SDNode<"TMSISD::CMPLTU", SDT_cmp, [SDNPHasChain]>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;

def fake_call_start : TMSInst<(outs), (ins i32imm:$val), "#callstart",
				[(callseq_start timm:$val)], UnitD>;
def fake_call_end : TMSInst<(outs), (ins i32imm:$val, i32imm:$val2), "#callend",
				[(callseq_end timm:$val, timm:$val2)], UnitD>;

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;
let isReturn = 1, isTerminator = 1 in {
	def ret : TMSInst<(outs), (ins), "br B3", [(retflag)], UnitS>;
}

def mv : TMSInst<(outs GPRegs:$out), (ins GPRegs:$in), "mv $in, $out", [],
				UnitD>;




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

// Match ints in the mvk range
def : Pat<(i32 mvk_pred:$val), (mvk imm:$val)>;

// mvkl/mvkh: doesn't look like llvm can express setting a bitfield in a
// register, and these two insns only set a halfword in one reg. So the only
// full way of expressing this would be to split gpregs into halfword regs.
// Instead, don't let mvkl/mvkh match anything, and use a pattern to match
// and set 32 bit constants
def mvkl : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvkl $val, $reg", [], UnitS>;

// For following pattern we have to output something (the reg loaded) but also
// need to input a register to set (from mvkl). So make this 2addr.
def mvkh : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val, GPRegs:$bees),
	"mvkh $val, $bees", [], UnitS> {
	let Constraints = "$bees = $reg";
}

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh mvk_all_pred:$val, (mvkl mvk_all_pred:$val))>;
def : Pat<(i32 (Wrapper tglobaladdr:$val)),
	(mvkh tglobaladdr:$val, (mvkl tglobaladdr:$val))>;

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

def ldw_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi32 idxaddr:$ptr))], UnitD>;

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


// Comparisons

def cmpeq : TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpeq $src1, $src2, $out",
		[(set GPRegs:$out, (cmpeq_node GPRegs:$src1, GPRegs:$src2))],
		UnitL>;

def cmpgt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgt $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgt_node GPRegs:$src1, GPRegs:$src2))],
		UnitL>;

def cmpgtu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgtu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgtu_node GPRegs:$src1, GPRegs:$src2))],
		UnitL>;

def cmplt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmplt $src1, $src2, $out",
		[(set GPRegs:$out, (cmplt_node GPRegs:$src1, GPRegs:$src2))],
		UnitL>;

def cmpltu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpltu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgt_node GPRegs:$src1, GPRegs:$src2))],
		UnitL>;

// Branch / call instructions

def ti_brcond : TMSInst<(outs), (ins brtarget:$block, nodefault_pred:$bees),
	"b $block", [(brcond_node bb:$block, predwrapper:$bees)], UnitS> {
	let InOperandList = (ins brtarget:$block, nodefault_pred:$bees);
}

def callp : TMSInst<(outs), (ins calltarget:$dst),
			"callp $dst", [(TMS320C64Xcall tglobaladdr:$dst)],
			UnitS> {
	let isCall = 1;
	let Defs = [A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A16, A17, A18, A19,
		A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
		B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B16, B17, B18, B19,
		B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31];
}
