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
def cmpeq_node : SDNode<"TMSISD::CMPEQ", SDTIntBinOp, [SDNPOutFlag]>;
def cmpgt_node : SDNode<"TMSISD::CMPGT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpgtu_node : SDNode<"TMSISD::CMPGTU", SDTIntBinOp, [SDNPOutFlag]>;
def cmplt_node : SDNode<"TMSISD::CMPLT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpltu_node : SDNode<"TMSISD::CMPLTU", SDTIntBinOp, [SDNPOutFlag]>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;

// XXX These should probably be marked as for codegen modelling only.
// Don't have flexibility to test right now though.
def fake_call_start : inst<(outs), (ins i32imm:$val), "#callstart",
		[(callseq_start timm:$val)], 0, unit_d>;
def fake_call_end : inst<(outs), (ins i32imm:$val, i32imm:$val2), "#callend",
		[(callseq_end timm:$val, timm:$val2)], 0, unit_d>;

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;

// FIXME: mv can be in any s/d/l slot
def mv : inst<(outs GPRegs:$out), (ins GPRegs:$in), "mv $in, $out", [], 0,
								unit_s>;


// Shifts and rotates

def srl_p_rr : pseudo_rr<"shr", unit_s, srl, 0>;
def srl_p_ri : pseudo_ri<"shr", (i32 uconst5:$imm), unit_s, srl, 0>;
def shl_p_rr : pseudo_rr<"shl", unit_s, shl, 0>;
def shl_p_ri : pseudo_ri<"shl", (i32 uconst5:$imm), unit_s, shl, 0>;

defm srl : inst_xr<"shr", unit_s, srl, 0>;
defm srl : inst_xi<"shr", (i32 uconst5:$imm), unit_s, srl, 0>;
defm shl : inst_xr<"shl", unit_s, shl, 0>;
defm shl : inst_xi<"shl", (i32 uconst5:$imm), unit_s, shl, 0>;


//Some arithmatic and logic

// Instr definition (spru732h p94) states that when printing we should use
// src1, src2 if the xform is used (src2), and src2, src1 if we use the xform
// and a constant.
// XXX - for unit D, depending on the form the constant can be sign extended
// or zero extended; potential for datasheet bugs or data entry bugs here
def add_p_rx : pseudo_rx<"add", unit_l, add, 0>;
def add_p_xr : pseudo_xr<"add", unit_l, add, 1>;
def add_p_xi : pseudo_xi<"add", (i32 sconst5:$imm), unit_l, add, 0>;

defm add_l : inst_rx<"add", unit_l, add, 0>;
defm add_l : inst_xr<"add", unit_l, add, 1>;
defm add_l : inst_xi<"add", (i32 sconst5:$imm), unit_l, add, 0>;
defm add_s : inst_rx<"add", unit_s, add, 0>;
defm add_s : inst_xr<"add", unit_s, add, 1>;
defm add_s : inst_xi<"add", (i32 sconst5:$imm), unit_s, add, 0>;
defm add_d : inst_rr<"add", unit_d, add, 0>;
defm add_d : inst_ri<"add", (i32 uconst5:$imm), unit_d, add, 0>;
defm add_d : inst_rx<"add", unit_d, add, 0>;
defm add_d : inst_xr<"add", unit_d, add, 1>;
defm add_d : inst_xi<"add", (i32 sconst5:$imm), unit_d, add, 0>;

let Constraints = "$src1 = $dst", AsmString = "addk $imm, $dst" in {
def addk_p : pseudo_ri<"addk", (i32 sconst16:$imm), unit_s, add, 0>;
defm addk : inst_ri<"addk", (i32 sconst16:$imm), unit_s, add, 0>;
}

def and_p_rx : pseudo_rx<"and", unit_l, and, 0>;
def and_p_xr : pseudo_xr<"and", unit_l, and, 1>;
def and_p_xi : pseudo_xi<"and", (i32 sconst5:$imm), unit_l, and, 1>;

defm and_l : inst_rx<"and", unit_l, and, 0>;
defm and_l : inst_xr<"and", unit_l, and, 1>;
defm and_l : inst_xi<"and", (i32 sconst5:$imm), unit_l, and, 1>;
defm and_s : inst_rx<"and", unit_s, and, 0>;
defm and_s : inst_xr<"and", unit_s, and, 1>;
defm and_s : inst_xi<"and", (i32 sconst5:$imm), unit_s, and, 1>;
defm and_d : inst_rx<"and", unit_d, and, 0>;
defm and_d : inst_xr<"and", unit_d, and, 1>;
defm and_d : inst_xi<"and", (i32 sconst5:$imm), unit_d, and, 1>;

def sub_p_rx : pseudo_rx<"sub", unit_l, sub, 0>;
def sub_p_xr : pseudo_xr<"sub", unit_l, sub, 0>;
def sub_p_xi : pseudo_ri<"sub", (i32 uconst5:$imm), unit_d, sub, 0>;

defm sub_l : inst_rx<"sub", unit_l, sub, 0>;
defm sub_l : inst_xr<"sub", unit_l, sub, 0>;
// l unit has a const form, but it's subtract-reg-from-const, which is
// pretty useless, but also won't be swallowed by tablegen
defm sub_s : inst_rx<"sub", unit_s, sub, 0>;
// s unit also has subtract-from-const form
defm sub_d : inst_rr<"sub", unit_d, sub, 0>;
defm sub_d : inst_ri<"sub", (i32 uconst5:$imm), unit_d, sub, 0>;
defm sub_d : inst_xr<"sub", unit_d, sub, 0>;

// Loading consts: almost always as halfwords. Put our own bodges on top
// of these instruction definitions - we want the simple specifications for
// pseudo and real instructions, but also to eliminate the register operand.
// We also don't want to pattern match anything, use explicit patterns for that

let InOperandList = (ins i32imm:$imm, pred:$p), AsmString = "mvk $imm, $dst" in{
def mvk_p : pseudo_rr<"mvk", unit_s, add, 0> {
	let Pattern = [];
}

defm mvk_s : inst_rr<"mvk", unit_s, add, 0>;
defm mvk_l : inst_rr<"mvk", unit_l, add, 0>;
defm mvk_d : inst_rr<"mvk", unit_d, add, 0>;
}

// Match ints in the mvk range
def : Pat<(i32 mvk_pred:$val), (mvk_p imm:$val)>;

// mvkl/mvkh: doesn't look like llvm can express setting a bitfield in a
// register, and these two insns only set a halfword in one reg. So the only
// full way of expressing this would be to split gpregs into halfword regs.
// Instead, don't let mvkl/mvkh match anything, and use a pattern to match
// and set 32 bit constants

let InOperandList = (ins i32imm:$imm, pred:$p), AsmString = "mvkl $imm, $dst" in
{
def mvkl_p : pseudo_rr<"mvkl", unit_s, add, 0> {
	let Pattern = [];
}
defm mvkl_s : inst_rr<"mvkl", unit_s, add, 0>;
}

let Constraints = "$src1 = $dst", AsmString = "mvkh $imm, $dst" in {
def mvkh_p : pseudo_ri<"mvkh", (ins i32imm:$imm), unit_s, add, 0> {
	let Pattern = [];
}
defm mvkh_s : inst_ri<"mvkh", (i32 i32imm:$imm), unit_s, add, 0>;
}

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh_p mvk_all_pred:$val, (mvkl_p mvk_all_pred:$val))>;
def : Pat<(i32 (Wrapper tglobaladdr:$val)),
	(mvkh_p tglobaladdr:$val, (mvkl_p tglobaladdr:$val))>;

// FIXME: Work out what on earth to do with lea
def lea_fail : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		"add ${ptr:facebees}, $dst", [(set GPRegs:$dst, idxaddr:$ptr)],
		0, unit_l>;

/*
// Loads: we can handle all post/pre inc/dec modes, and all indexing situations,
// with a) alignment limitation, b) offset limitation, c) sign extension
def ldb : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldb $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi8 addr:$ptr))]>;

def ldb_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldb $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi8 idxaddr:$ptr))]>;

def ldbu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldbu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi8 addr:$ptr))]>;

def ldbu_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldbu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi8 idxaddr:$ptr))]>;

// FIXME: Memory alignment ;_;
def ldh : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldh $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi16 addr:$ptr))]>;

def ldhu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldhu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi16 addr:$ptr))]>;

def ldw : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (load addr:$ptr))]>;

def ldw_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (load idxaddr:$ptr))]>;

def stb : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stb $reg, $ptr",
	[(truncstorei8 GPRegs:$reg, addr:$ptr)]>;

def stb_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stb $reg, $ptr",
	[(truncstorei8 GPRegs:$reg, idxaddr:$ptr)]>;

def sth : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"sth $reg, $ptr",
	[(truncstorei16 GPRegs:$reg, addr:$ptr)]>;

def stw : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, addr:$ptr)]>;

def stw_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, idxaddr:$ptr)]>;


// Comparisons

def cmpeq : TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpeq $src1, $src2, $out",
		[(set GPRegs:$out, (cmpeq_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpgt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgt $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgt_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpgtu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgtu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgtu_node GPRegs:$src1, GPRegs:$src2))]>;

def cmplt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmplt $src1, $src2, $out",
		[(set GPRegs:$out, (cmplt_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpltu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpltu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpltu_node GPRegs:$src1, GPRegs:$src2))]>;

// Branch / call instructions

def branch : TMSInst<(outs), (ins brtarget:$block), "b $block",
			[(br bb:$block)]>;

def ti_brcond : TMSInst<(outs), (ins brtarget:$block, nodefault_pred:$bees),
	"b $block", [(brcond_node bb:$block, predwrapper:$bees)]> {
	let InOperandList = (ins brtarget:$block, nodefault_pred:$bees);
}

def callp : TMSInst<(outs), (ins calltarget:$dst),
			"callp $dst", [(TMS320C64Xcall tglobaladdr:$dst)]> {
	let isCall = 1;
	let Defs = [A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A16, A17, A18, A19,
		A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
		B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B16, B17, B18, B19,
		B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31];
}

let isReturn = 1, isTerminator = 1 in {
	// NB: branch-to-register can only work on unit S2
	// Epilogue code should juggle the return addr into B3.
	def ret : inst<(outs), (ins), "br B3", [(retflag)], 1, unit_s>;
}
