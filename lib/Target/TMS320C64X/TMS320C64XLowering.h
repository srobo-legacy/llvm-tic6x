//==-- TMS320C64XLowering.h - TMS320C64X DAG Lowering Interface --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
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
class TMS320C64XSubtarget;
class TMS320C64XTargetMachine;

class TMS320C64XLowering : public TargetLowering {
public:
	explicit TMS320C64XLowering(TMS320C64XTargetMachine &TM);
	~TMS320C64XLowering();

	unsigned getFunctionAlignment(const Function *F) const;
	SDValue LowerFormalArguments(SDValue Chain,
				unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals);

	const TMS320C64XTargetMachine &TM;
};
} // namespace llvm

#endif // LLVM_TARGET_TMS320C64X_LOWERING_H
