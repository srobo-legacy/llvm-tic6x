//==-- TMS320C64XLowering.h - TMS320C64X DAG Lowering Interface --*- C++ -*-==//
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
		SELECT,
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
				CallingConv::ID CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);
	SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv,
				bool isVarArg,
				const SmallVectorImpl<ISD::OutputArg> &Outs,
				DebugLoc dl, SelectionDAG &DAG);
	SDValue LowerCall(SDValue Chain, SDValue Callee, CallingConv::ID
				CallConv, bool isVarArg, bool &isTailCall,
				const SmallVectorImpl<ISD::OutputArg> &Outs,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);
	SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
				CallingConv::ID CallConv, bool isVarArg,
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
	SDValue LowerSelect(SDValue op, SelectionDAG &DAG);
	SDValue LowerVASTART(SDValue op, SelectionDAG &DAG);
	SDValue LowerVAARG(SDValue op, SelectionDAG &DAG);

	const TMS320C64XTargetMachine &TM;
};
} // namespace llvm

#endif // LLVM_TARGET_TMS320C64X_LOWERING_H
