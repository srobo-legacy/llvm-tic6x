//===---- TMS320C64XSelector.cpp - Instruction selector for TMS320C64X ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#include "TMS320C64XSelector.h"
using namespace llvm;

TMS320C64XInstSelectorPass::TMS320C64XInstSelectorPass(
						TMS320C64XTargetMachine &TM)
	: SelectionDAGISel(TM)
{
}

void
TMS320C64XInstSelectorPass::InstructionSelect()
{

	llvm_unreachable_internal("Unimplemented function InstructionSelect()");
}
