//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
// Copyright 2010 Jeremy Morse <jeremy.morse@gmail.com>. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JEREMY MORSE ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL JEREMY MORSE OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//===----------------------------------------------------------------------===//

// misc, to begin with

def SDT_Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_brcond : SDTypeProfile<0, 3, [SDTCisVT<0, OtherVT>, SDTCisVT<1, i32>, SDTCisVT<2, i32>]>;
def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_call_seq_start : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_call_seq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_call_ret_label : SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>;
def SDT_call_ret_label_operand : SDTypeProfile<1, 1, [SDTCisVT<0, i32>,
							SDTCisVT<1, i32>]>;
def SDT_tmsselect : SDTypeProfile<1, 4, [SDTCisVT<0, i32>, SDTCisVT<1, i32>,
					SDTCisVT<2, i32>, SDTCisVT<3, i32>,
					SDTCisVT<4, i32>]>;

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
def tmsselect : SDNode<"TMSISD::SELECT", SDT_tmsselect>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;

// XXX These should probably be marked as for codegen modelling only.
// Don't have flexibility to test right now though.
def call_start_i : inst<(outs), (ins i32imm:$val), "sub\t.D2\tB15,\t$val,\tB15",
		[(callseq_start uconst5:$val)], 1, unit_d> {
	let Pattern = sched_pattern;
	let hasSideEffects = 1;
}

def call_start_r : inst<(outs), (ins GPRegs:$val), "sub\t.D2X\tB15,\t$val,\tB15"
		,[(callseq_start GPRegs:$val)], 1, unit_d> {
	let Pattern = sched_pattern;
	let hasSideEffects = 1;
}

def call_end_i : inst<(outs), (ins i32imm:$val, i32imm:$val2),
		"add\t.D2\t$val,\tB15,\tB15",
		[(callseq_end timm:$val, uconst5:$val2)], 1, unit_d> {
	let Pattern = sched_pattern;
	let hasSideEffects = 1;
}

def call_end_r : inst<(outs), (ins GPRegs:$val, i32imm:$val2),
		"add\t.D2X\tB15,\t$val,\tB15",
		[(callseq_end GPRegs:$val, timm:$val2)], 1, unit_d> {
	let Pattern = sched_pattern;
	let hasSideEffects = 1;
}

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;

// FIXME: mv can be in any s/d/l slot
let neverHasSideEffects = 1 in {
let AsmString = "mv\t\t$in,\t$out" in {
def mv : inst<(outs GPRegs:$out), (ins GPRegs:$in), "", [], 0, unit_s>;
def mv1 : inst<(outs GPRegs:$out), (ins GPRegs:$in), "", [], 0, unit_s>;
}

def mv2 : inst<(outs GPRegs:$out), (ins GPRegs:$in), "mv\t.S2\t$in,\t$out",
				 [], 1, unit_s>;

def noop : inst<(outs), (ins i32imm:$cycles), "nop\t.S1\t$cycles", [],
				0, unit_s>;
}

// Shifts and rotates

def srl_p_rr : pseudo_rr<"shru", ".S1", unit_s, srl, 0>;
def srl_p_ri : pseudo_ri<"shru", ".S1", (i32 uconst5:$imm), unit_s, srl, 0>;
def shl_p_rr : pseudo_rr<"shl", ".S1", unit_s, shl, 0>;
def shl_p_ri : pseudo_ri<"shl", ".S1", (i32 uconst5:$imm), unit_s, shl, 0>;
def shr_p_rr : pseudo_rr<"shr", ".S1", unit_s, sra, 0>;
def shr_p_ri : pseudo_ri<"shr", ".S1", (i32 uconst5:$imm), unit_s, sra, 0>;
def rotl_p_rr : pseudo_rr<"rotl", ".M1", unit_m, rotl, 0>;
def rotl_p_ri : pseudo_ri<"rotl", ".M1", (i32 uconst5:$imm), unit_m, rotl, 0>;

//Some arithmatic and logic

// Instr definition (spru732h p94) states that when printing we should use
// src1, src2 if the xform is used (src2), and src2, src1 if we use the xform
// and a constant.
// XXX - for unit D, depending on the form the constant can be sign extended
// or zero extended; potential for datasheet bugs or data entry bugs here
def add_p_rr : pseudo_rr<"add", ".L1", unit_l, add, 0>;
def add_p_ri : pseudo_ri<"add", ".L1", (i32 sconst5:$imm), unit_l, add, 0>;

let Constraints = "$src1 = $dst", AsmString = "addk\t.S1\t$imm,\t$dst" in {
def addk_p : pseudo_ri<"addk", "", (i32 sconst16:$imm), unit_s, add, 0>;
}

def and_p_rr : pseudo_rr<"and", ".L1", unit_l, and, 0>;
def and_p_ri : pseudo_ri<"and", ".L1", (i32 sconst5:$imm), unit_l, and, 1>;

def or_p_rr : pseudo_rr<"or", ".D1", unit_d, or, 0>;
def or_p_ri : pseudo_ri<"or", ".D1", (i32 sconst5:$imm), unit_d, or, 1>;

def xor_p_rr : pseudo_rr<"xor", ".D1", unit_d, xor, 0>;
def xor_p_ri : pseudo_ri<"xor", ".D1", (i32 sconst5:$imm), unit_d, xor, 1>;

def sub_p_rr : pseudo_rr<"sub", ".L1", unit_l, sub, 0>;
def sub_p_ri : pseudo_ri<"sub", ".D1", (i32 uconst5:$imm), unit_d, sub, 0>;

// Prolog asm requires some special cases:

// The actual prolog itself now requires some insns to occur in parallel,
// which I haven't yet modelled for tablegen. So instead we have this fake
// instruction that the assembly printer will see and emit the parallel code
// for. Unpleasent, but saves a whole load of pain immediately.
def prolog : inst<(outs), (ins i32imm:$stacksz), "", [], 0, unit_s>;

// Similar story for epilog
def epilog : inst<(outs), (ins), "", [], 0, unit_s>;

// Multiplies - there are a million and one of them, so we'll just be using
// mpy32. It might be possible to apply various constraints to the size of
// operands so that we can use 16 bit multiplies, but there's no chance of
// being able to account for different positions in registers.

// NB: mpy32 is c64x+ only

let delay_slots = 3, hasDelaySlot = 1 in {
def mpy32_p : pseudo_rr<"mpy32", ".M1", unit_m, mul, 0>;
}


// Loading consts: almost always as halfwords. Put our own bodges on top
// of these instruction definitions - we want the simple specifications for
// pseudo and real instructions, but also to eliminate the register operand.
// We also don't want to pattern match anything, use explicit patterns for that

let InOperandList = (ins i32imm:$imm, pred:$s),
			AsmString = "mvk\t.S1\t$imm,\t$dst" in {
def mvk_p : pseudo_rr<"mvk", ".S1", unit_s, add, 0> {
	let Pattern = [];
}
}

// Match ints in the mvk range
def : Pat<(i32 mvk_pred:$val), (mvk_p imm:$val)>;

// mvkl/mvkh: doesn't look like llvm can express setting a bitfield in a
// register, and these two insns only set a halfword in one reg. So the only
// full way of expressing this would be to split gpregs into halfword regs.
// Instead, don't let mvkl/mvkh match anything, and use a pattern to match
// and set 32 bit constants

let InOperandList = (ins i32imm:$imm, pred:$s),
		AsmString = "mvkl\t.S1\t$imm,\t$dst" in {
def mvkl_p : pseudo_rr<"mvkl", "", unit_s, add, 0> {
	let Pattern = [];
}
}

def mvkh_p : inst<(outs GPRegs:$dst), (ins i32imm:$imm, GPRegs:$src1),
		"mvkh\t.S1\t$imm,\t$dst", [], 0, unit_s> {
	let Constraints = "$src1 = $dst";
}

let Constraints = "$src1 = $dst" in {
def mvkh_1 : inst<(outs ARegs:$dst), (ins i32imm:$imm, ARegs:$src1),
		"mvkh\t.S1\t$imm,\t$dst", [], 0, unit_s>;
def mvkh_2 : inst<(outs BRegs:$dst), (ins i32imm:$imm, BRegs:$src1),
		"mvkh\t.S2\t$imm,\t$dst", [], 1, unit_s>;
}

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh_p mvk_all_pred:$val, (mvkl_p mvk_all_pred:$val))>;
def : Pat<(i32 (Wrapper tglobaladdr:$val)),
	(mvkh_p tglobaladdr:$val, (mvkl_p tglobaladdr:$val))>;
def : Pat<(i32 (Wrapper tjumptable:$dst)),
	(mvkh_p tjumptable:$dst, (mvkl_p tjumptable:$dst))>;


let InOperandList = (ins call_ret_label_operand_operand:$imm, pred:$s),
		AsmString = "mvkl\t.S1\t$imm,\t$dst" in {
def mvkl_label : pseudo_rr<"mvkl", "", unit_s, add, 0> {
	let Pattern = [];
}
}

def mvkh_label : inst<(outs GPRegs:$dst), (ins call_ret_label_operand_operand:$imm, GPRegs:$src1),
		"mvkh\t.S1\t$imm,\t$dst", [], 0, unit_s> {
	let Constraints = "$src1 = $dst";
}

def : Pat<(call_ret_label_operand mvk_all_pred:$label),
	(mvkh_label mvk_all_pred:$label, (mvkl_label mvk_all_pred:$label))>;

// FIXME: Work out what on earth to do with lea
def lea_fail : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		"add\t.L1\t$ptr,\t$dst",
		[(set GPRegs:$dst, idxaddr:$ptr)],
		0, unit_l> {
	let Pattern = sched_pattern;
}


// Memory access: we can handle all post/pre inc/dec modes, and all indexing
// situations, with a) alignment limitation, b) offset limitation,
// c) sign extension

let mem_shift_amt = 0 in {
defm u_i1 : meminst_load<"bu", zextloadi1>;
}

let mem_shift_amt = 0 in {
defm byte : meminst<"b", sextloadi8, truncstorei8>;
defm ubyte : meminst_load<"bu", zextloadi8>;
}

let mem_shift_amt = 1 in {
defm hword : meminst<"h", sextloadi16, truncstorei16>;
defm uhword : meminst_load<"hu", zextloadi16>;
}

let mem_shift_amt = 2 in {
defm word : meminst<"w", load, store>;
}

// Comparisons

def cmpeq_p_rr : pseudo_rr<"cmpeq", ".L1", unit_l, cmpeq_node, 0>;
def cmpeq_p_ri : pseudo_ri<"cmpeq", ".L1", (i32 sconst5:$imm), unit_l,
				cmpeq_node, 1>;

def : Pat<(cmpne_node GPRegs:$reg1, GPRegs:$reg2),
		(xor_p_ri (cmpeq_p_rr GPRegs:$reg1, GPRegs:$reg2), (i32 1))>;

// NOTE: Can't use any of the immediate forms of comparison instructions. LLVM
// always puts the constant operand on the right hand side of all operations,
// and TI instructions only allow constants on the left. That's fine and can
// be flipped, but that requires llvm to then go through all uses and invert
// the polarity of any tests/whatevers based on this. Not sure how to do that.
// Possibly it can't be done, we just have to handle it in some machdep fashion.

def cmpgt_p_rr : pseudo_rr<"cmpgt", ".L1", unit_l, cmpgt_node, 0>;
//def cmpgt_p_ri : pseudo_ri<"cmpgt", ".L1", (i32 sconst5:$imm), unit_l,
//						cmpgt_node, 1>;

def cmpgtu_p_rr : pseudo_rr<"cmpgtu", ".L1", unit_l, cmpgtu_node, 0>;
//def cmpgtu_p_ri : pseudo_ri<"cmpgtu", ".L1", (i32 uconst4:$imm), unit_l,
//						cmpgtu_node, 1>;

def cmplt_p_rr : pseudo_rr<"cmplt", ".L1", unit_l, cmplt_node, 0>;
//def cmplt_p_ri : pseudo_ri<"cmplt", ".L1", (i32 sconst5:$imm), unit_l,
//						cmplt_node, 1>;

def cmpltu_p_rr : pseudo_rr<"cmpltu", ".L1", unit_l, cmpltu_node, 0>;
//def cmpltu_p_ri : pseudo_ri<"cmpltu", ".L1", (i32 uconst4:$imm), unit_l,
//						cmpltu_node, 1>;

// Select - lowering loads one value into first reg, then we conditionally
// move the other value into it. Having a pair of instructions both move into
// the same register would achieve it with one cycle, but that involves touching
// parallel stuff, which we're avoiding for now

def mvselect : nopred_inst<(outs GPRegs:$dst), (ins GPRegs:$reg, GPRegs:$src1),
		"mv\t.S1\t$reg,\t$dst", [], 0, unit_s> {
	let Constraints = "$src1 = $dst";
}

def : Pat<(tmsselect GPRegs:$false, GPRegs:$true, timm:$trueimm, predwrapper1:$truereg),
		(mvselect GPRegs:$true, GPRegs:$false, timm:$trueimm, predwrapper1:$truereg)>;

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh_p mvk_all_pred:$val, (mvkl_p mvk_all_pred:$val))>;

// Branch / call instructions

let isTerminator = 1, isBranch = 1, delay_slots = 5, hasDelaySlot = 1 in {
def branch_p : inst<(outs), (ins brtarget:$block), "b\t.S1\t$block",
				[(br bb:$block)], 0, unit_s> {
	let Pattern = sched_pattern;
}
def branch_1 : inst<(outs), (ins brtarget:$block), "b\t.S1\t$block",
				[(br bb:$block)], 0, unit_s>;
def branch_2 : inst<(outs), (ins brtarget:$block), "b\t.S2\t$block",
				[(br bb:$block)], 1, unit_s>;

def brcond_p : nopred_inst<(outs), (ins brtarget:$block), "b\t.S1\t$block",
		[(brcond_node bb:$block, timm:$a, predwrapper1:$b)], 0, unit_s>{
	let Pattern = sched_pattern;
}

def bindirect_2 : inst<(outs), (ins GPRegs:$reg), "bnop\t.S2X\t$reg,\t5",
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

// NB: Only side 2 callp instructions can be used - this is because they store
// the return address in register 3, on whichever side the callp was executed
// upon. So callp .S1 puts ret in A3, which isn't consistent with the TI
// calling convention.

def callp_p : inst<(outs), (ins calltarget:$dst, variable_ops),
			"callp\t.S2\t$dst", [(TMS320C64Xcall tglobaladdr:$dst)],
			1, unit_s> {
	let Pattern = sched_pattern;
}

def callp_e_p : inst<(outs), (ins calltarget:$dst, variable_ops),
			"callp\t.S2\t$dst",
			[(TMS320C64Xcall texternalsym:$dst)], 1, unit_s> {
	let Pattern = sched_pattern;
}

def call_indirect_2 : inst<(outs), (ins GPRegs:$reg, variable_ops),
				"bnop\t.S2X\t$reg,\t5",
				[(TMS320C64Xcall GPRegs:$reg)],
				1, unit_s> {
	let Pattern = sched_pattern;
	let Uses = [B3];
}
}

// Hacks: the processor doesn't _actually_ have a real call instruction, callp
// was introduced by c64x+, and there's no support at all for indirect calls.
// So we have to work by loading the return address into B3 manually before
// branching to the target. Which means injecting a label immediately after the
// branch instruction itself. Which is what this is.
def return_label : inst<(outs), (ins i32imm:$imm), ".retaddr_$imm:",
				[(call_ret_label imm:$imm)], 0, unit_m> {
	let Pattern = sched_pattern;
}

// NB: branch-to-register can only work on unit S2
// Epilogue code should juggle the return addr into B3.
// We only define one instruction in this form, scheduling shouldn't
// have any reason touching this anyway.
def ret : inst<(outs), (ins), "bnop\t.S2\tB3,\t5", [(retflag)], 1, unit_s> {
	let isReturn = 1;
	let isTerminator = 1;
	let isPredicable = 0;
	let Pattern = sched_pattern;
}
