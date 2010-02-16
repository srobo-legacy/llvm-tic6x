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

#include "TMS320C64XTargetMachine.h"
#include "TMS320C64XRegisterInfo.h"
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
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
using namespace llvm;

namespace llvm {
#include "TMS320C64XGenCallingConv.inc"
}

TMS320C64XLowering::TMS320C64XLowering(TMS320C64XTargetMachine &tm) :
	TargetLowering(tm, new TargetLoweringObjectFileCOFF()), TM(tm)
{

	/* Ugh */
	addRegisterClass(MVT::i32, TMS320C64X::GPRegsRegisterClass);

	setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);
	/* All other loads have sx and zx support */

	/* No 32 bit load insn */
	setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);

	/* No in-reg sx */
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

	/* No divide anything */
	setOperationAction(ISD::UDIV, MVT::i32, Expand);
	setOperationAction(ISD::SDIV, MVT::i32, Expand);
	setOperationAction(ISD::UREM, MVT::i32, Expand);
	setOperationAction(ISD::SREM, MVT::i32, Expand);
	setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
	setOperationAction(ISD::SDIVREM, MVT::i32, Expand);

	/* Curious that llvm has a select; tms320c64x doesn't though, expanding
	 * apparently leads to SELECT_CC insns */
	setOperationAction(ISD::SELECT, MVT::i32, Expand);
	/* No setcc either, although we can emulate it */
	setOperationAction(ISD::SETCC, MVT::i32, Expand);
	/* No branchcc, but we have brcond */
	setOperationAction(ISD::BR_CC, MVT::i32, Expand);

	/* Probably is a membarrier, but I'm not aware of it right now */
	setOperationAction(ISD::MEMBARRIER, MVT::Other, Expand);

	/* Should also inject other invalid operations here */


	setStackPointerRegisterToSaveRestore(TMS320C64X::B30);

	computeRegisterProperties();
	return;
}

TMS320C64XLowering::~TMS320C64XLowering()
{

	/* _nothing_ */
	return;
}

unsigned
TMS320C64XLowering::getFunctionAlignment(Function const*) const
{

	return 5; /* 32 bytes; instruction packet */
}

SDValue
TMS320C64XLowering::LowerFormalArguments(SDValue Chain,
				unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals)
{
#if 0
	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	MachineRegisterInfo &RegInfo = MF.getRegInfo();
#endif

	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, getTargetMachine(), ArgLocs,
			*DAG.getContext());
	CCInfo.AnalyzeFormalArguments(Ins, CC_TMS320C64X);

}
