//===- TMS320C64XInstrFormats.td - TMS320C64X Insn formats -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
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

class InstUnit<int u> {
	int unit = u;
}

def UnitL	: InstUnit<0>;
def UnitS	: InstUnit<1>;
def UnitD	: InstUnit<2>;
def UnitM	: InstUnit<3>;

def mem_operand	: Operand<i32> {
	let PrintMethod = "printMemOperand";
	let MIOperandInfo = (ops GPRegs, GPRegs);
}

def calltarget : Operand<i32>;

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

def sconst5_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getSExtValue(), MVT::i32);
}]>;
def sconst5 : PatLeaf<(imm), [{
	int val = (int)N->getSExtValue();
	return (val <= 15 && val >= -16);
}], sconst5_xform>;

def uconst15_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getZExtValue(), MVT::i32);
}]>;
def uconst15 : PatLeaf<(imm), [{
	int val = (int)N->getZExtValue();
	return (val < 0x7FF);
}], uconst15_xform>;

def mvk_xform : SDNodeXForm<imm, [{
	return CurDAG->getTargetConstant((int)N->getSExtValue(), MVT::i32);
}]>;
def mvk_pred : PatLeaf<(imm), [{
	int val = (int)N->getSExtValue();
	return (val < 0x7FFF && val >= -0x8000);
}], mvk_xform>;

def mvk_all_pred : PatLeaf<(imm), [{
	return true;
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

class TMSInst<dag outs, dag ins, string mnemonic, list<dag> pattern, InstUnit u>
							: Instruction {
	// All insns are conditional unless stated otherwise
	let isPredicable = 1;

	let Namespace = "TMS320C64X";
	let OutOperandList = outs;
	let InOperandList = ins;
	let AsmString = mnemonic;
	let Pattern = pattern;
	InstUnit unit = u;
}
