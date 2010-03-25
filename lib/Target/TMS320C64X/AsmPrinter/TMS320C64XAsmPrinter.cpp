//===-- TMS320C64XAsmPrinter.cpp - TMS320C64X LLVM assembly writer --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "TMS320C64XInstrInfo.h"
#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XTargetMachine.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/DwarfWriter.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetAsmInfo.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Mangler.h"
#include "llvm/Support/MathExtras.h"
using namespace llvm;

namespace llvm {
	class TMS320C64XAsmPrinter : public AsmPrinter {
public:
	explicit TMS320C64XAsmPrinter(formatted_raw_ostream &O,
		TargetMachine &TM, const TargetAsmInfo *T, bool V);

	virtual const char *getPassName() const {
		return "TMS320C64X Assembly Printer";
	}

	bool print_predicate(const MachineInstr *MI);
	void printInstruction(const MachineInstr *MI);
	bool runOnMachineFunction(MachineFunction &F);
	void printUnitOperand(const MachineInstr *MI, int op);
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
					TargetMachine &TM,
					const TargetAsmInfo *T, bool V)
      : AsmPrinter(O, TM, T, V) 
{
}

bool
TMS320C64XAsmPrinter::runOnMachineFunction(MachineFunction &MF)
{
	Function *F = MF.getFunction();
	this->MF = &MF;

	SetupMachineFunction(MF);
	EmitConstantPool(MF.getConstantPool());
	O << "\n\n";
	EmitAlignment(F->getAlignment(), F);
	O << "\t.globl\t" << CurrentFnName << "\n";
	O << "\t" << CurrentFnName << ":\n";
	printVisibility(CurrentFnName, F->getVisibility());

	for (MachineFunction::const_iterator I = MF.begin(), E = MF.end();
								I != E; ++I) {
		if (I != MF.begin()) {
			printBasicBlockLabel(I, true, true);
			O << "\n";
		}

		for (MachineBasicBlock::const_iterator II = I->begin(),
				E = I->end(); II != E; ++II) {
			print_predicate(II);
			printInstruction(II);
		}
	}

	return false;
}

void
TMS320C64XAsmPrinter::printUnitOperand(const MachineInstr *MI, int op_num)
{
	int i, top;
	char n, u, t;
	bool contains_xpath;
	const TargetInstrDesc desc = MI->getDesc();

	// For /all/ instructions, print unit and side specifier - at some
	// point I might beat the assembler into not caring, but until then,
	// it's obligatory

	switch (GET_UNIT(desc.TSFlags)) {
		case TMS320C64XII::unit_l:
			u = 'L';
			break;
		case TMS320C64XII::unit_s:
			u = 'S';
			break;
		case TMS320C64XII::unit_d:
			u = 'D';
			break;
		case TMS320C64XII::unit_m:
			u = 'M';
			break;
		default:
			llvm_unreachable("unknown unit when printing insn");
	}

	if (GET_SIDE(desc.TSFlags) & TMS320C64XII::unit_2)
		n = '2';
	else
		n = '1';

	t = 0;
	if (desc.TSFlags & TMS320C64XII::is_memaccess) {
		unsigned reg;
		if (desc.TSFlags & TMS320C64XII::is_store) {
			const MachineOperand MO = MI->getOperand(2);
			assert(MO.isReg() && "src/dst of memory access is not "
						"a register");
			reg = MO.getReg();
		} else {
			const MachineOperand MO = MI->getOperand(0);
			assert(MO.isReg() && "src/dst of memory access is not "
						"a register");
			reg = MO.getReg();
		}

		if (findRegisterSide(reg, MF) == TMS320C64X::ARegsRegisterClass)
			t = '1';
		else
			t = '2';
	}

	// We can't tell whether something uses the xpath from the instruction
	// itself; instead look at registers
	top = MI->findFirstPredOperandIdx();
	if (top == -1)
		top = MI->getNumOperands();

	TargetRegisterClass *rc;
	if (desc.TSFlags & TMS320C64XII::unit_2)
		rc = TMS320C64X::BRegsRegisterClass;
	else
		rc = TMS320C64X::ARegsRegisterClass;

	contains_xpath = false;
	for (i = 0; i < top; ++i)
		if (MI->getOperand(i).isReg())
			if (findRegisterSide(MI->getOperand(i).getReg(), MF)
									!= rc)
				contains_xpath = true;

	O << ".";
	O << u;
	O << n;
	if (t != 0) {
		O << "T";
		O << t;
	} else if (contains_xpath) {
		O << "X";
	}

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

	O << "\t[" << c << RI.get(reg).AsmName << "]";
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
	std::string name = Mang->getMangledName(GVar);
	Constant *C = GVar->getInitializer();
	sz = td->getTypeAllocSize(C->getType());
	align = td->getPreferredAlignment(GVar);

	printVisibility(name, GVar->getVisibility());

	OutStreamer.SwitchSection(getObjFileLowering().SectionForGlobal(GVar,
								Mang, TM));

	if (C->isNullValue() && !GVar->hasSection()) {
		if (!GVar->isThreadLocal() &&
			(GVar->hasLocalLinkage() || GVar->isWeakForLinker())) {
			if (sz == 0)
				sz = 1;

// XXX - .local is only for ELF targets, we're using coff.
// Best case commenting this out changes nothing; worst case it polutes
// the global namespace and causes linking errors later on. Curses.
#if 0
			if (GVar->hasLocalLinkage())
				O << "\t.local " << name << "\n";
#endif

			O << TAI->getCOMMDirective() << name << "," << sz;
			if (TAI->getCOMMDirectiveTakesAlignment())
				O << "," << (1 << align);

			O << "\n";
			return;
		}
	}

	// Insert here - linkage foo. Requires: understanding linkage foo.

	// Alignment gets generated in byte form, however we need to emit it
	// in gas' bit form.
	align = Log2_32(align);

	EmitAlignment(align, GVar);

	if (TAI->hasDotTypeDotSizeDirective()) {
		O << "\t.type " << name << ",#object\n";
		O << "\t.size " << name << "," << sz << "\n";
	}

	O << name << ":\n";
	EmitGlobalConstant(C);
}

void
TMS320C64XAsmPrinter::printOperand(const MachineInstr *MI, int op_num)
{
	const MachineOperand &MO = MI->getOperand(op_num);
	const TargetRegisterInfo &RI = *TM.getRegisterInfo();
	switch(MO.getType()) {
	case MachineOperand::MO_Register:
		if (TargetRegisterInfo::isPhysicalRegister(MO.getReg()))
			O << RI.get(MO.getReg()).AsmName;
		else
			llvm_unreachable("Nonphysical register being printed");
		break;
	case MachineOperand::MO_Immediate:
		O << (int)MO.getImm();
		break;
	case MachineOperand::MO_MachineBasicBlock:
		printBasicBlockLabel(MO.getMBB());
		break;
	case MachineOperand::MO_GlobalAddress:
		O << Mang->getMangledName(MO.getGlobal());
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

	O << ".ponylabel_";
	O << (int)MI->getOperand(op_num).getImm();
	return;
}

void
TMS320C64XAsmPrinter::printMemOperand(const MachineInstr *MI, int op_num,
					const char *Modifier)
{

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
	printOperand(MI, op_num);

	// Don't print zero offset
	if (MI->getOperand(op_num+1).isImm() &&
				MI->getOperand(op_num+1).getImm() == 0)
		return;

	O << "(";
	printOperand(MI, op_num+1);
	O << ")";
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
