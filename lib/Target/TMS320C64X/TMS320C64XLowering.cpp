//===-- TMS320C64XLowering.cpp - TMS320C64X DAG Lowering Implementation  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#include "TMS320C64XLowering.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "llvm/GlobalVariable.h"
#include "llvm/GlobalAlias.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
using namespace llvm;

TMS320C64XLowering::TMS320C64xLowering(TMS320C64XTargetMachine &tm) :
	TargetLowering(tm, new TargetLoweringObjectFileCOFF()),
	Subtarget(*tm.getSubtargetImpl()), TM(tm)
{

	/* Again; implement later */
	return;
}
