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

class MemAcc<ValueType sz, string printMethod> : Operand<i32> {
	let PrintMethod = printMethod;
	let MIOperandInfo = (ops ptr_rc, i1, i8, GPRegs);
}

def i8mem	: MemAcc<i8, "printi8mem">;
def i16mem	: MemAcc<i16, "printi16mem">;
def i32mem	: MemAcc<i32, "printi32mem">;
def i64mem	: MemAcc<i64, "printi64mem">;

def AddrType : ComplexPattern<i32, 4, "AddrBees", [], []>;

// ~

class TMSInst<dag outs, dag ins, string mnemonic, list<dag> pattern, InstUnit u>
							: Instruction {
	let Namespace = "TMS320C64X";
	let OutOperandList = outs;
	let InOperandList = ins;
	let AsmString = mnemonic;
	let Pattern = pattern;
	InstUnit unit = u;
}