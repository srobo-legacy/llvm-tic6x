//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

// misc, to begin with

def SDT_Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_brcond : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>]>;
def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_call_seq_start : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_call_seq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_call_ret_label : SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>;
def SDT_call_ret_label_operand : SDTypeProfile<1, 1, [SDTCisVT<0, i32>,
							SDTCisVT<1, i32>]>;

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
def call_ret_label : SDNode<"TMSISD::CALL_RET_LABEL", SDT_call_ret_label,
				[SDNPHasChain, SDNPOptInFlag, SDNPOutFlag,
					SDNPSideEffect]>;
def call_ret_label_operand : SDNode<"TMSISD::CALL_RET_LABEL_OPERAND",
				SDT_call_ret_label_operand,
				[SDNPHasChain, SDNPOptInFlag, SDNPOutFlag,
					SDNPSideEffect]>;
def cmpeq_node : SDNode<"TMSISD::CMPEQ", SDTIntBinOp,
				[SDNPOutFlag, SDNPCommutative]>;
def cmpne_node : SDNode<"TMSISD::CMPNE", SDTIntBinOp,
				[SDNPOutFlag, SDNPCommutative]>;
def cmpgt_node : SDNode<"TMSISD::CMPGT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpgtu_node : SDNode<"TMSISD::CMPGTU", SDTIntBinOp, [SDNPOutFlag]>;
def cmplt_node : SDNode<"TMSISD::CMPLT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpltu_node : SDNode<"TMSISD::CMPLTU", SDTIntBinOp, [SDNPOutFlag]>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;

// XXX These should probably be marked as for codegen modelling only.
// Don't have flexibility to test right now though.
def fake_call_start : inst<(outs), (ins i32imm:$val), "#callstart",
		[(callseq_start timm:$val)], 0, unit_d> {
	let Pattern = sched_pattern;
}
def fake_call_end : inst<(outs), (ins i32imm:$val, i32imm:$val2), "#callend",
		[(callseq_end timm:$val, timm:$val2)], 0, unit_d> {
	let Pattern = sched_pattern;
}

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;

// FIXME: mv can be in any s/d/l slot
def mv : inst<(outs GPRegs:$out), (ins GPRegs:$in), "mv\t$s\t$in,\t$out", [],
							0, unit_s>;

def mv1 : inst<(outs GPRegs:$out), (ins GPRegs:$in), "mv\t$s\t$in,\t$out", [],
							0, unit_s>;
def mv2 : inst<(outs GPRegs:$out), (ins GPRegs:$in), "mv\t$s\t$in,\t$out", [],
							1, unit_s>;

def noop : inst<(outs), (ins i32imm:$cycles), "nop\t$s\t$cycles", [], 0,unit_s>;


// Shifts and rotates

def srl_p_rr : pseudo_rr<"shr", unit_s, srl, 0>;
def srl_p_ri : pseudo_ri<"shr", (i32 uconst5:$imm), unit_s, srl, 0>;
def shl_p_rr : pseudo_rr<"shl", unit_s, shl, 0>;
def shl_p_ri : pseudo_ri<"shl", (i32 uconst5:$imm), unit_s, shl, 0>;
def shr_p_rr : pseudo_rr<"shr", unit_s, sra, 0>;
def shr_p_ri : pseudo_ri<"shr", (i32 uconst5:$imm), unit_s, sra, 0>;

defm srl : inst_xr<"shr", unit_s, srl, 0>;
defm srl : inst_xi<"shr", (i32 uconst5:$imm), unit_s, srl, 0>;
defm shl : inst_xr<"shl", unit_s, shl, 0>;
defm shl : inst_xi<"shl", (i32 uconst5:$imm), unit_s, shl, 0>;
defm shr : inst_xr<"shr", unit_s, sra, 0>;
defm shr : inst_xi<"shr", (i32 uconst5:$imm), unit_s, sra, 0>;


//Some arithmatic and logic

// Instr definition (spru732h p94) states that when printing we should use
// src1, src2 if the xform is used (src2), and src2, src1 if we use the xform
// and a constant.
// XXX - for unit D, depending on the form the constant can be sign extended
// or zero extended; potential for datasheet bugs or data entry bugs here
def add_p_rr : pseudo_rr<"add", unit_l, add, 0>;
def add_p_ri : pseudo_ri<"add", (i32 sconst5:$imm), unit_l, add, 0>;

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

let Constraints = "$src1 = $dst", AsmString = "addk\t$s\t$imm,\t$dst" in {
def addk_p : pseudo_ri<"addk", (i32 sconst16:$imm), unit_s, add, 0>;
defm addk : inst_ri<"addk", (i32 sconst16:$imm), unit_s, add, 0>;
}

def and_p_rr : pseudo_rr<"and", unit_l, and, 0>;
def and_p_ri : pseudo_ri<"and", (i32 sconst5:$imm), unit_l, and, 1>;

defm and_l : inst_rx<"and", unit_l, and, 0>;
defm and_l : inst_xr<"and", unit_l, and, 1>;
defm and_l : inst_xi<"and", (i32 sconst5:$imm), unit_l, and, 1>;
defm and_s : inst_rx<"and", unit_s, and, 0>;
defm and_s : inst_xr<"and", unit_s, and, 1>;
defm and_s : inst_xi<"and", (i32 sconst5:$imm), unit_s, and, 1>;
defm and_d : inst_rx<"and", unit_d, and, 0>;
defm and_d : inst_xr<"and", unit_d, and, 1>;
defm and_d : inst_xi<"and", (i32 sconst5:$imm), unit_d, and, 1>;

def or_p_rr : pseudo_rr<"or", unit_d, or, 0>;
def or_p_ri : pseudo_ri<"or", (i32 sconst5:$imm), unit_d, or, 0>;

defm or_d : inst_rx<"or", unit_d, or, 0>;
defm or_d : inst_xi<"or", (i32 sconst5:$imm), unit_d, or, 1>;
defm or_l : inst_rx<"or", unit_l, or, 0>;
defm or_l : inst_xi<"or", (i32 sconst5:$imm), unit_l, or, 1>;
defm or_s : inst_rx<"or", unit_s, or, 0>;
defm or_s : inst_xi<"or", (i32 sconst5:$imm), unit_s, or, 1>;

def xor_p_rr : pseudo_rr<"xor", unit_d, xor, 0>;
def xor_p_ri : pseudo_ri<"xor", (i32 sconst5:$imm), unit_d, xor, 0>;

defm xor_d : inst_rx<"xor", unit_d, xor, 0>;
defm xor_d : inst_xi<"xor", (i32 sconst5:$imm), unit_d, xor, 1>;
defm xor_l : inst_rx<"xor", unit_l, xor, 0>;
defm xor_l : inst_xi<"xor", (i32 sconst5:$imm), unit_l, xor, 1>;
defm xor_s : inst_rx<"xor", unit_s, xor, 0>;
defm xor_s : inst_xi<"xor", (i32 sconst5:$imm), unit_s, xor, 1>;

def sub_p_rr : pseudo_rr<"sub", unit_l, sub, 0>;
def sub_p_ri : pseudo_ri<"sub", (i32 uconst5:$imm), unit_d, sub, 0>;

defm sub_l : inst_rx<"sub", unit_l, sub, 0>;
defm sub_l : inst_xr<"sub", unit_l, sub, 0>;
// l unit has a const form, but it's subtract-reg-from-const, which is
// pretty useless, but also won't be swallowed by tablegen
defm sub_s : inst_rx<"sub", unit_s, sub, 0>;
// s unit also has subtract-from-const form
defm sub_d : inst_rr<"sub", unit_d, sub, 0>;
defm sub_d : inst_ri<"sub", (i32 uconst5:$imm), unit_d, sub, 0>;
defm sub_d : inst_xr<"sub", unit_d, sub, 0>;


// Multiplies - there are a million and one of them, so we'll just be using
// mpy32. It might be possible to apply various constraints to the size of
// operands so that we can use 16 bit multiplies, but there's no chance of
// being able to account for different positions in registers.

// NB: mpy32 is c64x+ only

let delay_slots = 3 in {
def mpy32_p : pseudo_rr<"mpy32", unit_m, mul, 0>;

defm mpy32 : inst_rr<"mpy32", unit_m, mul, 0>;
defm mpy32 : inst_rx<"mpy32", unit_m, mul, 0>;
defm mpy32 : inst_xr<"mpy32", unit_m, mul, 0>;
}


// Loading consts: almost always as halfwords. Put our own bodges on top
// of these instruction definitions - we want the simple specifications for
// pseudo and real instructions, but also to eliminate the register operand.
// We also don't want to pattern match anything, use explicit patterns for that

let InOperandList = (ins i32imm:$imm, pred:$s),
			AsmString = "mvk\t$s\t$imm,\t$dst" in {
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

let InOperandList = (ins i32imm:$imm, pred:$s),
		AsmString = "mvkl\t$s\t$imm,\t$dst" in {
def mvkl_p : pseudo_rr<"mvkl", unit_s, add, 0> {
	let Pattern = [];
}
defm mvkl_s : inst_rr<"mvkl", unit_s, add, 0>;
}

def mvkh_p : inst<(outs GPRegs:$dst), (ins i32imm:$imm, GPRegs:$src1),
		"mvkh\t$s\t$imm,\t$dst", [], 0, unit_s> {
	let Constraints = "$src1 = $dst";
}

let Constraints = "$src1 = $dst" in {
def mvkh_1 : inst<(outs ARegs:$dst), (ins i32imm:$imm, ARegs:$src1),
		"mvkh\t$s\t$imm,\t$dst", [], 0, unit_s>;
def mvkh_2 : inst<(outs BRegs:$dst), (ins i32imm:$imm, BRegs:$src1),
		"mvkh\t$s\t$imm,\t$dst", [], 1, unit_s>;
}

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh_p mvk_all_pred:$val, (mvkl_p mvk_all_pred:$val))>;
def : Pat<(i32 (Wrapper tglobaladdr:$val)),
	(mvkh_p tglobaladdr:$val, (mvkl_p tglobaladdr:$val))>;
def : Pat<(i32 (Wrapper tjumptable:$dst)),
	(mvkh_p tjumptable:$dst, (mvkl_p tjumptable:$dst))>;



let InOperandList = (ins call_ret_label_operand_operand:$imm, pred:$s),
		AsmString = "mvkl\t$s\t$imm,\t$dst" in {
def mvkl_label : pseudo_rr<"mvkl", unit_s, add, 0> {
	let Pattern = [];
}
}

def mvkh_label : inst<(outs GPRegs:$dst), (ins call_ret_label_operand_operand:$imm, GPRegs:$src1),
		"mvkh\t$s\t$imm,\t$dst", [], 0, unit_s> {
	let Constraints = "$src1 = $dst";
}

def : Pat<(call_ret_label_operand mvk_all_pred:$label),
	(mvkh_label mvk_all_pred:$label, (mvkl_label mvk_all_pred:$label))>;

// FIXME: Work out what on earth to do with lea
def lea_fail : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		"add\t$s\t${ptr:facebees},\t$dst",
		[(set GPRegs:$dst, idxaddr:$ptr)],
		0, unit_l> {
	let Pattern = sched_pattern;
}


// Memory access: we can handle all post/pre inc/dec modes, and all indexing
// situations, with a) alignment limitation, b) offset limitation,
// c) sign extension

defm byte_addr : meminst<"b", sextloadi8, truncstorei8, (i32 addr:$ptr)>;
defm byte_idx : meminst<"b", sextloadi8, truncstorei8, (i32 idxaddr:$ptr)>;
defm ubyte_addr : meminst_load<"bu", zextloadi8, (i32 addr:$ptr)>;
defm ubyte_idx : meminst_load<"bu", zextloadi8, (i32 idxaddr:$ptr)>;

defm hword_addr : meminst<"h", sextloadi16, truncstorei16, (i32 addr:$ptr)>;
defm hword_idx : meminst<"h", sextloadi16, truncstorei16, (i32 idxaddr:$ptr)>;
defm uhword_addr : meminst_load<"hu", zextloadi16, (i32 addr:$ptr)>;
defm uhword_idx : meminst_load<"hu", zextloadi16, (i32 idxaddr:$ptr)>;

defm word_addr : meminst<"w", load, store, (i32 addr:$ptr)>;
defm word_idx : meminst<"w", load, store, (i32 idxaddr:$ptr)>;

// Comparisons

def cmpeq_p_rr : pseudo_rr<"cmpeq", unit_l, cmpeq_node, 0>;
def cmpeq_p_ri : pseudo_ri<"cmpeq", (i32 sconst5:$imm), unit_l, cmpeq_node, 1>;
defm cmpeq : inst_rx<"cmpeq", unit_l, cmpeq_node, 0>;
defm cmpeq : inst_xr<"cmpeq", unit_l, cmpeq_node, 1>;
defm cmpeq : inst_xi<"cmpeq", (i32 sconst5:$imm), unit_l, cmpeq_node, 1>;

def : Pat<(cmpne_node GPRegs:$reg1, GPRegs:$reg2),
		(xor_p_ri (cmpeq_p_rr GPRegs:$reg1, GPRegs:$reg2), (i32 1))>;

def cmpgt_p_rr : pseudo_rr<"cmpgt", unit_l, cmpgt_node, 0>;
def cmpgt_p_ri : pseudo_ri<"cmpgt", (i32 sconst5:$imm), unit_l, cmpgt_node, 1>;
defm cmpgt : inst_rx<"cmpgt", unit_l, cmpgt_node, 0>;
defm cmpgt : inst_xr<"cmpgt", unit_l, cmpgt_node, 1>;
defm cmpgt : inst_xi<"cmpgt", (i32 sconst5:$imm), unit_l, cmpgt_node, 1>;

def cmpgtu_p_rr : pseudo_rr<"cmpgtu", unit_l, cmpgtu_node, 0>;
def cmpgtu_p_ri : pseudo_ri<"cmpgtu", (i32 uconst4:$imm), unit_l, cmpgtu_node,
									1>;
defm cmpgtu : inst_rx<"cmpgtu", unit_l, cmpgtu_node, 0>;
defm cmpgtu : inst_xr<"cmpgtu", unit_l, cmpgtu_node, 1>;
defm cmpgtu : inst_xi<"cmpgtu", (i32 uconst4:$imm), unit_l, cmpgtu_node, 1>;

def cmplt_p_rr : pseudo_rr<"cmplt", unit_l, cmplt_node, 0>;
def cmplt_p_ri : pseudo_ri<"cmplt", (i32 sconst5:$imm), unit_l, cmplt_node, 1>;
defm cmplt : inst_rx<"cmplt", unit_l, cmplt_node, 0>;
defm cmplt : inst_xr<"cmplt", unit_l, cmplt_node, 1>;
defm cmplt : inst_xi<"cmplt", (i32 sconst5:$imm), unit_l, cmplt_node, 1>;

def cmpltu_p_rr : pseudo_rr<"cmpltu", unit_l, cmpltu_node, 0>;
def cmpltu_p_ri : pseudo_ri<"cmpltu", (i32 uconst4:$imm), unit_l, cmpltu_node,
									1>;
defm cmpltu : inst_rx<"cmpltu", unit_l, cmpltu_node, 0>;
defm cmpltu : inst_xr<"cmpltu", unit_l, cmpltu_node, 1>;
defm cmpltu : inst_xi<"cmpltu", (i32 uconst4:$imm), unit_l, cmpltu_node, 1>;

// Branch / call instructions

let isBranch = 1, delay_slots = 4 in {
def branch_p : inst<(outs), (ins brtarget:$block), "b\t$s\t$block",
				[(br bb:$block)], 0, unit_s> {
	let Pattern = sched_pattern;
}
def branch_1 : inst<(outs), (ins brtarget:$block), "b\t$s\t$block",
				[(br bb:$block)], 0, unit_s>;
def branch_2 : inst<(outs), (ins brtarget:$block), "b\t$s\t$block",
				[(br bb:$block)], 1, unit_s>;


let InOperandList = (ins brtarget:$block, nodefault_pred:$s) in {
def brcond_p : inst<(outs), (ins), "b\t$s\t$block",
		[(brcond_node bb:$block, predwrapper:$s)], 0, unit_s> {
	let Pattern = sched_pattern;
}
def brcond_1 : inst<(outs), (ins), "b\t$s\t$block",
		[(brcond_node bb:$block, predwrapper:$bees)], 0, unit_s>;
def brcond_2 : inst<(outs), (ins), "b\t$s\t$block",
		[(brcond_node bb:$block, predwrapper:$bees)], 1, unit_s>;
}

def bindirect_2 : inst<(outs), (ins GPRegs:$reg), "b\t$s\t$reg",
				[(brind GPRegs:$reg)], 1, unit_s> {
	let Pattern = sched_pattern;
}
} // isBranch

// XXX XXX XXX: callp is a c64x+ instruction, which the assembler is unlikely
// to swallow right now. How does TI do branches-with-return-addr on c64x?
let isCall = 1, isPredicable = 0,
	Defs = [A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A16, A17, A18, A19,
		A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
		B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B16, B17, B18, B19,
		B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31] in {
def callp_p : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
			[(TMS320C64Xcall tglobaladdr:$dst)], 0, unit_s> {
	let Pattern = sched_pattern;
}
def callp_1 : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
		[(TMS320C64Xcall tglobaladdr:$dst)], 0, unit_s>;
def callp_2 : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
		[(TMS320C64Xcall tglobaladdr:$dst)], 1, unit_s>;

def callp_e_p : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
			[(TMS320C64Xcall texternalsym:$dst)], 0, unit_s> {
	let Pattern = sched_pattern;
}
def callp_e_1 : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
		[(TMS320C64Xcall texternalsym:$dst)], 0, unit_s>;
def callp_e_2 : inst<(outs), (ins calltarget:$dst), "callp\t$s\t$dst",
		[(TMS320C64Xcall texternalsym:$dst)], 1, unit_s>;

def call_indirect_2 : inst<(outs), (ins BRegs:$reg), "b\t$s\t$reg",
				[(TMS320C64Xcall BRegs:$reg)], 1, unit_s> {
	let Pattern = sched_pattern;
	let Uses = [B3];
}
}

// Hacks: the processor doesn't _actually_ have a real call instruction, callp
// was introduced by c64x+, and there's no support at all for indirect calls.
// So we have to work by loading the return address into B3 manually before
// branching to the target. Which means injecting a label immediately after the
// branch instruction itself. Which is what this is.
def return_label : inst<(outs), (ins i32imm:$imm), ".ponylabel_$imm:",
				[(call_ret_label imm:$imm)], 0, unit_m> {
	let Pattern = sched_pattern;
}

// NB: branch-to-register can only work on unit S2
// Epilogue code should juggle the return addr into B3.
// We only define one instruction in this form, scheduling shouldn't
// have any reason touching this anyway.
def ret : inst<(outs), (ins), "bnop\t$s\tB3,\t5", [(retflag)], 1, unit_s> {
	let isReturn = 1;
	let isTerminator = 1;
	let isPredicable = 0;
	let Pattern = sched_pattern;
}
