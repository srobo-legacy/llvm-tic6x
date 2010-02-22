//===-- TMS320C64XAsmPrinter.cpp - TMS320C64X LLVM assembly writer --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//
//===----------------------------------------------------------------------===//

#include "TMS320C64X.h"
#include "TMS320C64XInstrInfo.h"
#include "TMS320C64XTargetMachine.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/DwarfWriter.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Target/TargetAsmInfo.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Mangler.h"
using namespace llvm;

namespace llvm {
	class TMS320C64XAsmPrinter : public AsmPrinter {
public:
	explicit TMS320C64XAsmPrinter(formatted_raw_ostream &O,
		TargetMachine &TM, const TargetAsmInfo *T, bool V);

	virtual const char *getPassName() const {
		return "TMS320C64X Assembly Printer";
	}

	void printInstruction(const MachineInstr *MI);
	bool runOnMachineFunction(MachineFunction &F);
	void PrintGlobalVariable(const GlobalVariable *GVar);
	void printOperand(const MachineInstr *MI, int opNum);
	void printMemOperand(const MachineInstr *MI, int opNum,
					const char *Modifier = 0);
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

	SetupMachineFunction(MF);
	EmitConstantPool(MF.getConstantPool());
	O << "\n\n";
	EmitAlignment(F->getAlignment(), F);
	O << "\t.globl\t" << CurrentFnName << "\n";
	printVisibility(CurrentFnName, F->getVisibility());

	for (MachineFunction::const_iterator I = MF.begin(), E = MF.end();
								I != E; ++I) {
		if (I != MF.begin()) {
			printBasicBlockLabel(I, true, true);
			O << "\n";
		}

		for (MachineBasicBlock::const_iterator II = I->begin(),
				E = I->end(); II != E; ++II) {
			printInstruction(II);
		}
	}

	return false;
}

void
TMS320C64XAsmPrinter::PrintGlobalVariable(const GlobalVariable *GVar)
{

	llvm_unreachable_internal("Unimplemented function PrintGlobalVariable");
}

void
TMS320C64XAsmPrinter::printOperand(const MachineInstr *MI, int op_num)
{
	if (op_num >= MI->getNumOperands()) {
		O << "Bees";
		return;
	}
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
	case MachineOperand::MO_ConstantPoolIndex:
	default:
		llvm_unreachable("Unknown operand type");
	}
}

void
TMS320C64XAsmPrinter::printMemOperand(const MachineInstr *MI, int op_num,
					const char *Modifier)
{

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
