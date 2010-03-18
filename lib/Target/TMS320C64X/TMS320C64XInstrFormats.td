//===- TMS320C64XInstrFormats.td - TMS320C64X Insn formats -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
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
			(ops (i32 -1), (i32 FakeReg))> {
	// Predicate of -1 indicates that this is always-execute
	let PrintMethod= "printUnitOperand";
}

def nodefault_pred : PredicateOperand<OtherVT, (ops timm, PredRegs),
			(ops)> {
	let PrintMethod = "printUnitOperand";
}

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



class inst<dag outs, dag ins, string mnemonic, list<dag> pattern, bit side,
			InstUnit u> : Instruction {
	// All insns are conditional unless stated otherwise
	let isPredicable = 1;

	let Namespace = "TMS320C64X";
	let OutOperandList = outs;
	let InOperandList = !con(ins, (ops pred:$s));
	let AsmString = mnemonic;

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
	bit mem_load_store = 0; // 0 for load, 1 for store
}

class pseudo_rr<string mnemonic, InstUnit u, SDNode op, int swap> :
		inst<(outs ARegs:$dst), (ins ARegs:$src1, ARegs:$src2), "",
		[(set ARegs:$dst, (op ARegs:$src1, ARegs:$src2))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
			let Pattern = sched_pattern;
		}

class pseudo_ri<string mnemonic, dag imm, InstUnit u, SDNode op, int swap> :
		inst<(outs ARegs:$dst), (ins ARegs:$src1, i32imm:$imm), "",
		[(set ARegs:$dst, (op ARegs:$src1, imm))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$imm,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$imm,\t$dst"));
			let Pattern = sched_pattern;
		}

multiclass meminst_load<string mnemonic, PatFrag op, dag ptr> {
	def _load_p : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		!strconcat("ld",!strconcat(mnemonic, "\t$s\t$ptr,\t$dst")),
		[(set GPRegs:$dst, (op ptr))], 0, unit_d> {
			let Pattern = sched_pattern;
			let delay_slots = 4;
			let mem_access = 1;
		}

	def _load1 : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		!strconcat("ld",!strconcat(mnemonic, "\t$s\t$ptr,\t$dst")),
		[(set GPRegs:$dst, (op ptr))], 0, unit_d> {
			let delay_slots = 4;
			let mem_access = 1;
		}
	def _load2 : inst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		!strconcat("ld",!strconcat(mnemonic, "\t$s\t$ptr,\t$dst")),
		[(set GPRegs:$dst, (op ptr))], 1, unit_d> {
			let delay_slots = 4;
			let mem_access = 1;
		}
}

multiclass meminst_store<string mnemonic, PatFrag op, dag ptr> {
	def _store_p : inst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		!strconcat("st",!strconcat(mnemonic, "\t$s\t$reg,\t$ptr")),
		[(op GPRegs:$reg, ptr)], 0, unit_d> {
			let Pattern = sched_pattern;
			let delay_slots = 4;
			let mem_access = 1;
			let mem_load_store = 1;
		}

	def _store1 : inst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		!strconcat("st",!strconcat(mnemonic, "\t$s\t$reg,\t$ptr")),
		[(op GPRegs:$reg, ptr)], 0, unit_d> {
			let delay_slots = 4;
			let mem_access = 1;
			let mem_load_store = 1;
		}
	def _store2 : inst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
		!strconcat("st",!strconcat(mnemonic, "\t$s\t$reg,\t$ptr")),
		[(op GPRegs:$reg, ptr)], 1, unit_d> {
			let delay_slots = 4;
			let mem_access = 1;
			let mem_load_store = 1;
		}
}

multiclass meminst<string mnemonic, PatFrag loadop, PatFrag storeop, dag addr> :
	meminst_load<mnemonic, loadop, addr>,
	meminst_store<mnemonic, storeop, addr>;

// Plain double register operand
multiclass inst_rr<string mnemonic, InstUnit u, SDNode op, int swap> {
	def _rr1 : inst<(outs ARegs:$dst), (ins ARegs:$src1, ARegs:$src2), "",
		[(set ARegs:$dst, (op ARegs:$src1, ARegs:$src2))],
			0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$src1,\t$src2,\t$dst"));
		}
	def _rr2 : inst<(outs BRegs:$dst), (ins BRegs:$src1, BRegs:$src2), "",
		[(set BRegs:$dst, (op BRegs:$src1, BRegs:$src2))], 1, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
		}
}

// Double register, second is xpath
// Note that I'd love to add an extra level of indirection to reduce replication
// but it doesn't look like tablegen can stitch dags together at that level.
multiclass inst_rx<string mnemonic, InstUnit u, SDNode op, int swap> {
	def _rx1 : inst<(outs ARegs:$dst), (ins ARegs:$src1, GPRegs:$src2), "",
		[(set ARegs:$dst, (op ARegs:$src1, GPRegs:$src2))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
		}
	def _rx2 : inst<(outs BRegs:$dst), (ins BRegs:$src1, GPRegs:$src2), "",
		[(set BRegs:$dst, (op BRegs:$src1, GPRegs:$src2))], 1, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
		}
}

multiclass inst_xr<string mnemonic, InstUnit u, SDNode op, int swap> {
	def _xr1 : inst<(outs ARegs:$dst), (ins GPRegs:$src1, ARegs:$src2), "",
		[(set ARegs:$dst, (op GPRegs:$src1, ARegs:$src2))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
		}
	def _xr2 : inst<(outs BRegs:$dst), (ins GPRegs:$src1, BRegs:$src2), "",
		[(set BRegs:$dst, (op GPRegs:$src1, BRegs:$src2))], 1, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$src2,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$src2,\t$dst"));
		}
}

// One register, one immediate
multiclass inst_ri<string mnemonic, dag imm, InstUnit u, SDNode op, int swap> {
	def _ri1 : inst<(outs ARegs:$dst), (ins ARegs:$src1, i32imm:$imm), "",
		[(set ARegs:$dst, (op ARegs:$src1, imm))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$imm,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$imm,\t$dst"));
		}
	def _ri2 : inst<(outs BRegs:$dst), (ins BRegs:$src1, i32imm:$imm), "",
		[(set BRegs:$dst, (op BRegs:$src1, imm))], 1, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$imm,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$imm,\t$dst"));
		}
}

// One xpath reg, one imm
multiclass inst_xi<string mnemonic, dag imm, InstUnit u, SDNode op, int swap> {
	def _xi1 : inst<(outs ARegs:$dst), (ins GPRegs:$src1, i32imm:$imm), "",
		[(set ARegs:$dst, (op GPRegs:$src1, imm))], 0, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$imm,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$imm,\t$dst"));
		}
	def _xi2 : inst<(outs BRegs:$dst), (ins GPRegs:$src1, i32imm:$imm), "",
		[(set BRegs:$dst, (op GPRegs:$src1, imm))], 1, u> {
			let AsmString = !if(swap,
			!strconcat(mnemonic, "\t$s\t$imm,\t$src1,\t$dst"),
			!strconcat(mnemonic, "\t$s\t$src1,\t$imm,\t$dst"));
		}
}
