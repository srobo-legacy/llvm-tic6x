//==-- TMS320C64XLowering.h - TMS320C64X DAG Lowering Interface --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_LOWERING_H
#define LLVM_TARGET_TMS320C64X_LOWERING_H

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Target/TargetLoweringObjectFile.h"

namespace llvm {
namespace TMSISD {
	enum {
		FIRST_NUMBER = ISD::BUILTIN_OP_END,
		BRCOND,
		CALL,
		CALL_RET_LABEL,
		CALL_RET_LABEL_OPERAND,
		CMPEQ,
		CMPNE,
		CMPGT,
		CMPGTU,
		CMPLT,
		CMPLTU,
		RET_FLAG,
		WRAPPER
	};
}
class TMS320C64XSubtarget;
class TMS320C64XTargetMachine;

class TMS320C64XLowering : public TargetLowering {
public:
	explicit TMS320C64XLowering(TMS320C64XTargetMachine &TM);
	~TMS320C64XLowering();

	unsigned getFunctionAlignment(const Function *F) const;
	const char *getTargetNodeName(unsigned op) const;
	SDValue LowerFormalArguments(SDValue Chain,
				unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);
	SDValue LowerReturn(SDValue Chain, unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::OutputArg> &Outs,
				DebugLoc dl, SelectionDAG &DAG);
	SDValue LowerCall(SDValue Chain, SDValue Callee, unsigned CallConv,
				bool isVarArg, bool isTailCall,
				const SmallVectorImpl<ISD::OutputArg> &Outs,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);
	SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
				unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);
	SDValue LowerOperation(SDValue op, SelectionDAG &DAG);
	SDValue LowerGlobalAddress(SDValue op, SelectionDAG &DAG);
	SDValue LowerJumpTable(SDValue op, SelectionDAG &DAG);
	SDValue LowerExtLoad(SDValue op, SelectionDAG &DAG);
	SDValue LowerReturnAddr(SDValue op, SelectionDAG &DAG);
	SDValue LowerBRCC(SDValue op, SelectionDAG &DAG);
	SDValue LowerSETCC(SDValue op, SelectionDAG &DAG);

	const TMS320C64XTargetMachine &TM;
};
} // namespace llvm

#endif // LLVM_TARGET_TMS320C64X_LOWERING_H
