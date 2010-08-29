//===-- TMS320C64XAsmPrinter.cpp - TMS320C64X LLVM assembly writer --------===//
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

#include "TMS320C64X.h"
#include "TMS320C64XInstrInfo.h"
#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XTargetMachine.h"
#include "TMS320C64XMCAsmInfo.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/DwarfWriter.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/ADT/SmallString.h"
using namespace llvm;

namespace llvm {
	class TMS320C64XAsmPrinter : public AsmPrinter {
public:
	explicit TMS320C64XAsmPrinter(formatted_raw_ostream &O,
		TargetMachine &TM, MCContext &Ctx, MCStreamer &Streamer,
		const MCAsmInfo *Asm);

	virtual const char *getPassName() const {
		return "TMS320C64X Assembly Printer";
	}

	const char *getRegisterName(unsigned RegNo);

	bool print_predicate(const MachineInstr *MI);
	void printInstruction(const MachineInstr *MI);
	void emit_prolog(const MachineInstr *MI);
	void emit_epilog(const MachineInstr *MI);
	bool runOnMachineFunction(MachineFunction &F);
	void PrintGlobalVariable(const GlobalVariable *GVar);
	void printOperand(const MachineInstr *MI, int opNum);
	void printMemOperand(const MachineInstr *MI, int opNum,
					const char *Modifier = 0);
	void printRetLabel(const MachineInstr *MI, int opNum);
	void printCCOperand(const MachineInstr *MI, int opNum);

	bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
		unsigned AsmVariant, const char *ExtraCode);
	bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
		unsigned AsmVariant, const char *ExtraCode);
};
}

#include "TMS320C64XGenAsmWriter.inc"

TMS320C64XAsmPrinter::TMS320C64XAsmPrinter(formatted_raw_ostream &O,
					TargetMachine &TM, MCContext &Ctx,
					MCStreamer &Streamer,
					const MCAsmInfo *Asm)
      : AsmPrinter(O, TM, Ctx, Streamer, Asm)
{
}

bool
TMS320C64XAsmPrinter::runOnMachineFunction(MachineFunction &MF)
{
	Function *F = MF.getFunction();
	this->MF = &MF;

	SetupMachineFunction(MF);
	EmitConstantPool();
	O << "\n\n";
	EmitAlignment(F->getAlignment(), F);

	EmitFunctionHeader();

	// Due to having to beat predecates manually, we don't use
	// EmitFunctionBody, but instead pump out instructions manually
	for (MachineFunction::const_iterator I = MF.begin(), E = MF.end();
								I != E; ++I) {
		if (I != MF.begin()) {
			EmitBasicBlockStart(I);
			O << "\n";
		}

		for (MachineBasicBlock::const_iterator II = I->begin(),
				E = I->end(); II != E; ++II) {
			unsigned opcode;

			opcode = II->getDesc().getOpcode();
			if (opcode == TMS320C64X::prolog) {
				emit_prolog(II);
			} else if (opcode == TMS320C64X::epilog) {
				emit_epilog(II);
			} else {
				print_predicate(II);
				printInstruction(II);
			}
			O << "\n";
		}
	}

	return false;
}

void
TMS320C64XAsmPrinter::emit_prolog(const MachineInstr *MI)
{
	// See instr info td file for why we do this here

	O << "\t\tmvk\t\t";
	printOperand(MI, 0);
	O << ",\tA0\n";
	O << "\t||\tmv\t\tB15,\tA1\n";
	O << "\t\tstw\t\tA15,\t*B15\n";
	O << "\t||\tstw\t\tB3,\t*-A1(4)\n";
	O << "\t||\tmv\t\tB15,\tA15\n";
	O << "\t||\tsub\t\tB15,\tA0\t,B15\n";

	return;
}
void
TMS320C64XAsmPrinter::emit_epilog(const MachineInstr *MI)
{
	// See instr info td file for why we do this here

	O << "\t\tldw\t\t*-A15(4),\tB3\n";
	O << "\t\tmv\t\tA15,\tB15\n";
	O << "\t||\tldw\t\t*A15,\tA15\n";
	O << "\t\tnop\t\t4\n";

	return;
}
bool
TMS320C64XAsmPrinter::print_predicate(const MachineInstr *MI)
{
	int pred_idx, nz, reg;
	char c;
	const TargetRegisterInfo &RI = *TM.getRegisterInfo();

	// Can't use first predicate operand any more, due to unit_operand hack
	pred_idx = MI->findFirstPredOperandIdx();

	if (!MI->getDesc().isPredicable())
		return false;

	if (pred_idx == -1) {
		// No predicate here
		O << "\t";
		return false;
	}

	nz = MI->getOperand(pred_idx).getImm();
	reg = MI->getOperand(pred_idx+1).getReg();

	if (nz == -1) {
		// This isn't a predicate
		O << "\t";
		return false;
	}


	if (nz)
		c = ' ';
	else
		c = '!';

	if (!TargetRegisterInfo::isPhysicalRegister(reg))
		llvm_unreachable("Nonphysical register used for predicate");

	O << "\t[" << c << RI.getName(reg) << "]";
	return true;
}

void
TMS320C64XAsmPrinter::PrintGlobalVariable(const GlobalVariable *GVar)
{
	unsigned int align, sz;
	const TargetData *td = TM.getTargetData();

	// Comments elsewhere say we discard this because external globals
	// require no code; why do we have to do that here though?
	if (!GVar->hasInitializer())
		return;

	if (EmitSpecialLLVMGlobal(GVar))
		return;

	O << "\n\n";

	SmallString<60> NameStr;
	Mang->getNameWithPrefix(NameStr, GVar, false);
	Constant *C = GVar->getInitializer();
	sz = td->getTypeAllocSize(C->getType());
	align = td->getPreferredAlignment(GVar);

	OutStreamer.SwitchSection(getObjFileLowering().SectionForGlobal(GVar,
								Mang, TM));

	if (C->isNullValue() && !GVar->hasSection()) {
		if (!GVar->isThreadLocal() &&
			(GVar->hasLocalLinkage() || GVar->isWeakForLinker())) {
			if (sz == 0)
				sz = 1;

			// XXX - .lcomm?
			O << NameStr << "," << sz;

			O << "\n";
			return;
		}
	}

	// Insert here - linkage foo. Requires: understanding linkage foo.

	// Alignment gets generated in byte form, however we need to emit it
	// in gas' bit form.
	align = Log2_32(align);

	EmitAlignment(align, GVar);

	if (MAI->hasDotTypeDotSizeDirective()) {
		O << "\t.type " << NameStr << ",#object\n";
		O << "\t.size " << NameStr << "," << sz << "\n";
	}

	O << NameStr << ":\n";
	EmitGlobalConstant(C);
}

void
TMS320C64XAsmPrinter::printOperand(const MachineInstr *MI, int op_num)
{
	SmallString<60> NameStr;
	MCSymbol *sym;
	const MachineOperand &MO = MI->getOperand(op_num);
	const TargetRegisterInfo &RI = *TM.getRegisterInfo();

	switch(MO.getType()) {
	case MachineOperand::MO_Register:
		if (TargetRegisterInfo::isPhysicalRegister(MO.getReg()))
			O << RI.getName(MO.getReg());
		else
			llvm_unreachable("Nonphysical register being printed");
		break;
	case MachineOperand::MO_Immediate:
		O << (int)MO.getImm();
		break;
	case MachineOperand::MO_MachineBasicBlock:
		sym = MO.getMBB()->getSymbol(OutContext);
		O << (*sym);
		break;
	case MachineOperand::MO_GlobalAddress:
		Mang->getNameWithPrefix(NameStr, MO.getGlobal(), false);
		O << NameStr;
		break;
	case MachineOperand::MO_ExternalSymbol:
		O << MO.getSymbolName();
		break;
	case MachineOperand::MO_JumpTableIndex:
		O << (int)MO.getIndex();
		break;
	case MachineOperand::MO_ConstantPoolIndex:
	default:
		llvm_unreachable("Unknown operand type");
	}
}

void
TMS320C64XAsmPrinter::printRetLabel(const MachineInstr *MI, int op_num)
{

	O << ".retaddr_";
	O << (int)MI->getOperand(op_num).getImm();
	return;
}

void
TMS320C64XAsmPrinter::printMemOperand(const MachineInstr *MI, int op_num,
					const char *Modifier)
{
	int offset;

	if (MI->getDesc().getOpcode() == TMS320C64X::lea_fail) {
		// I can't find a reasonable way to bounce a memory addr
		// calculation into normal operands (1 -> 2), so hack
		// this instead
		printOperand(MI, op_num);
		O << ",";
		O << '\t';
		printOperand(MI, op_num+1);
		return;
	}

	O << "*";

	// We may need to put a + or - in front of the base register to indicate
	// what we plan on doing with the constant
	if (MI->getOperand(op_num+1).isImm()) {
		offset = MI->getOperand(op_num+1).getImm();
		if (offset < 0)
			O << "-";
		else if (offset > 0)
			O << "+";
	}

	// Base register
	printOperand(MI, op_num);

	// Don't print zero offset, and if it's an immediate always print
	// a positive offset */
	if (MI->getOperand(op_num+1).isImm()) {
		if (offset != 0) {
			O << "(";
			O << abs(offset);
			O << ")";
		}
	} else {
		O << "(";
		printOperand(MI, op_num+1);
		O << ")";
	}

	return;
}

void
TMS320C64XAsmPrinter::printCCOperand(const MachineInstr *MI, int opNum)
{

	llvm_unreachable_internal("Unimplemented function printCCOperand");
}

bool
TMS320C64XAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
		unsigned AsmVariant, const char *ExtraCode)
{

	llvm_unreachable_internal("Unimplemented function PrintAsmOperand");
}

bool
TMS320C64XAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
						unsigned OpNo,
						unsigned AsmVariant,
						const char *ExtraCode)
{

	llvm_unreachable_internal("Unimplemented func PrintAsmMemoryOperand");
}

extern "C" void LLVMInitializeTMS320C64XAsmPrinter()
{

	RegisterAsmPrinter<TMS320C64XAsmPrinter> X(TheTMS320C64XTarget);
}
