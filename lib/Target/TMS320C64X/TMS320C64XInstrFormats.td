//===- TMS320C64XInstrFormats.td - TMS320C64X Insn formats -*- tablegen -*-===//
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

// Memory index, ie register, immediate. Note no restriction on imm size here
class MemIndex {
	bit is_imm = 0;
	int imm = -1;
	MainReg reg;
}

class MemRegIndex<MainReg r> : MemIndex {
	let is_imm = 0;
	let reg = r;
}

class MemImmIndex<int val> : MemIndex {
	let is_imm = 1;
	let imm = val;
}

class AddrMode<int kind> {
	int mode = kind;
}

def PreInc	: AddrMode<0>;
def PostInc	: AddrMode<1>;
def PreDec	: AddrMode<2>;
def PostDec	: AddrMode<3>;
def PreIncReg	: AddrMode<4>;
def PostIncReg	: AddrMode<5>;
def PreDecReg	: AddrMode<6>;
def PostDecReg	: AddrMode<7>;
def PreIncImm	: AddrMode<8>;
def PostIncImm	: AddrMode<9>;
def PreDecImm	: AddrMode<10>;
def PostDecImm	: AddrMode<11>;
def PlainReg	: AddrMode<12>;
def PlainImm	: AddrMode<13>;

class InstUnit<bits<2> u> {
	bits<2> unit = u;
}

def unit_d	: InstUnit<0>;
def unit_s	: InstUnit<1>;
def unit_l	: InstUnit<2>;
def unit_m	: InstUnit<3>;

def mem_operand	: Operand<i32> {
	let PrintMethod = "printMemOperand";
	let MIOperandInfo = (ops GPRegs, GPRegs);
}

def calltarget : Operand<i32>;
def brtarget : Operand<OtherVT>;
def call_ret_label_operand_operand : Operand<i32> {
	let PrintMethod = "printRetLabel";
}

def i8index : PatLeaf<(imm), [{
	return ((int)N->getSExtValue() >= 0 && (int)N->getSExtValue() < 32);
}]>;

def i16index : PatLeaf<(imm), [{
	return ((int)N->getSExtValue() >= 0 && (int)N->getSExtValue() < 64 &&
			((int)N->getSExtValue() & 1) == 0);
}]>;

def i32index : PatLeaf<(imm), [{
	return ((int)N->getSExtValue() >= 0 && (int)N->getSExtValue() < 32 &&
			((int)N->getSExtValue() & 3) == 0);
}]>;

def addr : ComplexPattern<i32, 2, "select_addr", [], []>;
def idxaddr : ComplexPattern<i32, 2, "select_idxaddr", [frameindex], []>;
def addr_d1t1 : ComplexPattern<i32, 2, "select_addr_d1t1", [], []>;
def addr_d2t2 : ComplexPattern<i32, 2, "select_addr_d2t2", [], []>;
def addr_d1t2 : ComplexPattern<i32, 2, "select_addr_d1t2", [], []>;
def addr_d2t1 : ComplexPattern<i32, 2, "select_addr_d2t1", [], []>;

def uconst4_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getSExtValue(), MVT::i32);
}]>;
def uconst4 : PatLeaf<(imm), [{
	int val = (int)N->getSExtValue();
	return (val <= 15 && val >= 0);
}], uconst4_xform>;

def sconst5_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getSExtValue(), MVT::i32);
}]>;
def sconst5 : PatLeaf<(imm), [{
	int val = (int)N->getSExtValue();
	return (val <= 15 && val >= -16);
}], sconst5_xform>;

def uconst5_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getZExtValue(), MVT::i32);
}]>;
def uconst5 : PatLeaf<(imm), [{
	int val = (int)N->getZExtValue();
	return (val >= 0 && val < 32);
}], uconst5_xform>;

def uconst15_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getZExtValue(), MVT::i32);
}]>;
def uconst15 : PatLeaf<(imm), [{
	int val = (int)N->getZExtValue();
	return (val < 0x7FF);
}], uconst15_xform>;

def sconst16_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getZExtValue(), MVT::i32);
}]>;
def sconst16 : PatLeaf<(imm), [{
	int val = (int)N->getZExtValue();
	return (val >= -0x800 && val < 0x7FF);
}], sconst16_xform>;

def mvk_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getSExtValue(), MVT::i32);
}]>;
def mvk_pred : PatLeaf<(imm), [{
	int val = (int)N->getSExtValue();
	return (val < 0x7FFF && val >= -0x8000);
}], mvk_xform>;

def mvk_all_pred : PatLeaf<(imm), [{
	return (N == N); // Avoid unused N warning
}], NOOP_SDNodeXForm>;

// ~

def set_lo : PatLeaf<(imm), [{
	unsigned int val = N->getZExtValue();
	return ((val & 0xFFFF) == val);
}]>;

def set_hi : PatLeaf<(imm), [{
	unsigned int val = (unsigned)N->getZExtValue();
	return ((val & 0xFFFF0000) == val);
}]>;

def pred : PredicateOperand<OtherVT, (ops timm, PredRegs),
			(ops (i32 -1), (i32 zero_reg))> {
	// Predicate of -1 indicates that this is always-execute
}

def nodefault_pred : PredicateOperand<OtherVT, (ops timm, PredRegs),
			(ops)>;

def nodefault_zflag : PredicateOperand<i32, (ops timm), (ops)>;
def nodefault_reg : PredicateOperand<i32, (ops PredRegs), (ops)>;

def predwrapper1 : ComplexPattern<i32, 1, "bounce_predicate", [], []>;

// A note on how we're formatting instruction representation.
//
// All instructions we care about right now have at _least_ a destination
// register operand, almost always one source register operand, generally two,
// sometimes an immediate instead. In general, instructions can only access
// registers on one side of the processor. Which side they're on is defined by
// where the destination register is (you can only write back to your own side).
//
// So for many instructions, all registers used will have to be on one side,
// hence a lot of the templating below orientates about all registers being in
// either ARegs or BRegs, but not both.
//
// Some instructions can use the "crosspath" (xpath), and access both banks of
// registers - but, only one insn on each side of the processor in any cycle.
// So to cater for those some insns have one operand as GPRegs rather than
// ARegs or BRes
//
// UPDATE: I'm covered in bees
//
// Some significant suckage has been averted at the expense of my eyes. Add to
// the above the fact that, to allow immediates we need to inject a dag
// specifying the predicate into the pattern. We also need to move the pattern
// into the multiclass, to allow side-specific register classes to be used.
//
// Also throw into this control over what order the operands match the operator
// node, and that this might have to be flipped to support what the assembler
// expects.
//
// So what we have, is death in a cake. Solution: pass in a dag for imms,
// the SDNode of the operator (neat trick from sparcs instrformats.td),
// and a "swap" integer operand, which selects whether the operand order is
// reversed when emmitted. Will it work, who can say.
//
// UPDATE: Tactical clarinet eye-strike
//
// Tablegen won't permit multiple instructions to match the same pattern, which
// torpedoes trying to generate VLIW parallelism at the instruction selection
// level. New tactic: we have a high-level pseudo-instruction, that represents
// all the different forms the insn can take. This only executes on one side of
// the processor, and only in one slot.
//
// We then have a set of "real" instructions, that represent the different forms
// and sides and units the instruction can have. We then have the option of
// using the singled-sided, non-parallel implementation. Or setting the
// usesCustomDAGSchedInserter flag on the psuedo-insts, and morphing them
// into parallel {unit,side} specific insns. Only the former will be implemented
// for now, the latter is an area of expansion that can be targetted when I'm
// not actually on fire.
//
// Implementing the latter will require two things - making a tablegen backend
// that spits out code to find what insns are valid to map the pseudo onto. In
// its current form tablegen output isn't sufficient to do this. After that, a
// register pass is needed to ensure correct legalisation of register
// allocation. AKA, making sure that the xpath is only used once per execution
// packet.



class inst<dag outs, dag ins, string asm_str, list<dag> pattern, bit side,
			InstUnit u> : Instruction {
	// All insns are conditional unless stated otherwise
	let isPredicable = 1;
	let CodeSize = 4;

	let Namespace = "TMS320C64X";
	let OutOperandList = outs;
	let InOperandList = !con(ins, (ops pred:$s));
	let AsmString = asm_str;

	// Match nothing; set sched_pattern for a future tablegen backend
	// to play with. pseudo instrs over-ride this behavior.
	list<dag> sched_pattern = pattern;
	let Pattern = [];

	InstUnit unit = u;
	bits<2> unit_bits = unit.unit;
	bit unit_no = side;
	bits<3> delay_slots = 0; // Range of 0 -> 4

	// Used to indicate memory datapath specifier, if this is memory access
	bit mem_access = 0;
	bits<2> mem_shift_amt = 0;
	bit mem_load_store = 0; // 0 for load, 1 for store
}

class nopred_inst<dag outs, dag ins, string mnemonic, list<dag> pattern,
				bit side, InstUnit u> :
		inst<outs, ins, mnemonic, pattern, side, u> {
	let InOperandList = !con(ins, (ops nodefault_zflag:$a, nodefault_reg:$b));
}

class memload<dag outs, dag ins, list<dag> pattern, bit side, InstUnit u> :
		inst<outs, ins, "", pattern, side, u> {
	let delay_slots = 4;
	let hasDelaySlot = 1;
	let mem_access = 1;
	let mem_load_store = 0;
}

class memstore<dag outs, dag ins, list<dag> pattern, bit side, InstUnit u> :
		inst<outs, ins, "", pattern, side, u> {
	let mem_access = 1;
	let mem_load_store = 1;
}

class pseudo_rr<string mnemonic, string sidespec, InstUnit u, SDNode op,
				int swap> :
		inst<(outs ARegs:$dst), (ins ARegs:$src1, ARegs:$src2), "",
		[(set ARegs:$dst, (op ARegs:$src1, ARegs:$src2))], 0, u> {
			string comb_mnemonic = !strconcat(!strconcat(mnemonic,
							"\t"), sidespec);
			let AsmString = !if(swap,
			!strconcat(comb_mnemonic,"\t$src2,\t$src1,\t$dst"),
			!strconcat(comb_mnemonic, "\t$src1,\t$src2,\t$dst"));
			let Pattern = sched_pattern;
		}

class pseudo_ri<string mnemonic, string sidespec, dag imm, InstUnit u,
				SDNode op, int swap> :
		inst<(outs ARegs:$dst), (ins ARegs:$src1, i32imm:$imm), "",
		[(set ARegs:$dst, (op ARegs:$src1, imm))], 0, u> {
			string comb_mnemonic = !strconcat(!strconcat(mnemonic,
							"\t"), sidespec);
			let AsmString = !if(swap,
			!strconcat(comb_mnemonic, "\t$imm,\t$src1,\t$dst"),
			!strconcat(comb_mnemonic, "\t$src1,\t$imm,\t$dst"));
			let Pattern = sched_pattern;
		}

multiclass meminst_load<string mnemonic, PatFrag op> {
	def _load_p_addr : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op addr:$ptr))], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D1T1\t$ptr,\t$dst"));
		}
	def _load_p_idx : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op idxaddr:$ptr))], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D1T1\t$ptr,\t$dst"));
	}

	def _load1 : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op addr_d1t1:$ptr))], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D1T1\t$ptr,\t$dst"));
		}
	def _load2 : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op addr_d2t2:$ptr))], 1, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D2T2\t$ptr,\t$dst"));
		}
	def _load1x : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op addr_d1t2:$ptr))], 0, unit_d> {
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D1T2\t$ptr,\t$dst"));
		}
	def _load2x : memload<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		[(set GPRegs:$dst, (op addr_d2t1:$ptr))], 1, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("ld",!strconcat(mnemonic,
						"\t.D2T1\t$ptr,\t$dst"));
		}
}

multiclass meminst_store<string mnemonic, PatFrag op> {
	def _store_p_addr : memstore<(outs), (ins mem_operand:$ptr,GPRegs:$reg),
		[(op GPRegs:$reg, addr:$ptr)], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D1T1\t$reg,\t$ptr"));
		}

	def _store_p_idxaddr : memstore<(outs), (ins mem_operand:$ptr,
		GPRegs:$reg), [(op GPRegs:$reg, idxaddr:$ptr)], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D1T1\t$reg,\t$ptr"));
	}

	def _store1 : memstore<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		[(op GPRegs:$reg, addr_d1t1:$ptr)], 0, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D1T1\t$reg,\t$ptr"));
		}
	def _store2 : memstore<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		[(op GPRegs:$reg, addr_d2t2:$ptr)], 1, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D2T2\t$reg,\t$ptr"));
		}
	def _store1x : memstore<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		[(op GPRegs:$reg, addr_d1t2:$ptr)], 0, unit_d> {
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D1T2\t$reg,\t$ptr"));
		}
	def _store2x : memstore<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		[(op GPRegs:$reg, addr_d2t1:$ptr)], 1, unit_d> {
			let Pattern = sched_pattern;
			let AsmString = !strconcat("st",!strconcat(mnemonic,
						"\t.D2T1\t$reg,\t$ptr"));
		}
}

multiclass meminst<string mnemonic, PatFrag loadop, PatFrag storeop> :
	meminst_load<mnemonic, loadop>,
	meminst_store<mnemonic, storeop>;
