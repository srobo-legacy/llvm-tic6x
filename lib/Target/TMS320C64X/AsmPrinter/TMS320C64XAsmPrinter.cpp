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
#if 0
    void printOperand(const MachineInstr *MI, int opNum);
    void printMemOperand(const MachineInstr *MI, int opNum,
                         const char *Modifier = 0);
    void printCCOperand(const MachineInstr *MI, int opNum);

    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       unsigned AsmVariant, const char *ExtraCode);
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             unsigned AsmVariant, const char *ExtraCode);
#endif
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

extern "C" void LLVMInitializeTMS320C64XAsmPrinter()
{

	RegisterAsmPrinter<TMS320C64XAsmPrinter> X(TheTMS320C64XTarget);
}
