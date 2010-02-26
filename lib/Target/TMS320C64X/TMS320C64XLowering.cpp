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
#include "TMS320C64XInstrInfo.h"
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
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/ADT/VectorExtras.h"
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
	SmallVector<CCValAssign, 16> ArgLocs;
	unsigned int i;

	MachineFunction &MF = DAG.getMachineFunction();
//	MachineRegisterInfo &RegInfo = MF.getRegInfo();
	MachineFrameInfo *MFI = MF.getFrameInfo();

	CCState CCInfo(CallConv, isVarArg, getTargetMachine(), ArgLocs,
			*DAG.getContext());
	CCInfo.AnalyzeFormalArguments(Ins, CC_TMS320C64X);

	for (i = 0; i < ArgLocs.size(); ++i) {
		CCValAssign &VA = ArgLocs[i];
		EVT ObjectVT = VA.getValVT();
		if (VA.isRegLoc()) {
			EVT RegVT = VA.getLocVT();
			unsigned Reg = MF.addLiveIn(VA.getLocReg(), &TMS320C64X::GPRegsRegClass);
			SDValue Arg = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);
			if (ObjectVT != MVT::i32) {
				/* Looks like AssertSext gets put in
				 * here topoint out the caller will
				 * have sign extended the incoming
				 * value */
				Arg = DAG.getNode(ISD::AssertSext, dl, MVT::i32,
					Arg, DAG.getValueType(ObjectVT));
				Arg = DAG.getNode(ISD::TRUNCATE, dl, 
							ObjectVT, Arg);
			}
			InVals.push_back(Arg);
		} else {
			EVT ValVT;
			if (VA.getLocInfo() == CCValAssign::Indirect)
				ValVT = VA.getLocVT();
			else
				ValVT = VA.getValVT();

			int FI = MFI->CreateFixedObject(ValVT.getSizeInBits()/8,
						VA.getLocMemOffset(), true);
			SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
			SDValue Load = DAG.getLoad(MVT::i32, dl, Chain, FIN,
							NULL, 0);
			InVals.push_back(Load);
		}
	}

	// XXX - varargs have to go on stack, and to match TI calling
	// convention the previous argument has to go on stack too.
	if (isVarArg) {
		llvm_unreachable_internal("No varargs yet, pls");
	}

	return Chain;
}

SDValue
TMS320C64XLowering::LowerReturn(SDValue Chain, unsigned CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                DebugLoc dl, SelectionDAG &DAG)
{
	SmallVector<CCValAssign, 16> RLocs;
	SDValue Flag;
	unsigned int i;

	CCState CCInfo(CallConv, isVarArg, DAG.getTarget(), RLocs,
						*DAG.getContext());
	CCInfo.AnalyzeReturn(Outs, RetCC_TMS320C64X);

	// Apparently we need to add this to the out list only if it's first
	if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
		for (i = 0; i != RLocs.size(); ++i)
			if (RLocs[i].isRegLoc())
				DAG.getMachineFunction().getRegInfo().addLiveOut
							(RLocs[i].getLocReg());
	}

	for (i = 0; i != RLocs.size(); ++i) {
		CCValAssign &VA = RLocs[i];
		assert(VA.isRegLoc() && "Invalid return position");

		Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), Outs[i].Val,
									Flag);

		Flag = Chain.getValue(1); // From sparc... why?
	}

	if (Flag.getNode())
		return DAG.getNode(TMSISD::RET_FLAG, dl, MVT::Other, Chain,
									Flag);
	return DAG.getNode(TMSISD::RET_FLAG, dl, MVT::Other, Chain);
}

SDValue
TMS320C64XLowering::LowerCall(SDValue Chain, SDValue Callee, unsigned CallConv,
			bool isVarArg, bool isTailCall,
			const SmallVectorImpl<ISD::OutputArg> &Outs,
			const SmallVectorImpl<ISD::InputArg> &Ins,
			DebugLoc dl, SelectionDAG &DAG,
			SmallVectorImpl<SDValue> &InVals)
{

// XXX XXX XXX - TI Calling convention dictates that the last argument before
// a series of vararg values on the stack must also be on the stack.

	unsigned int bytes, i;
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, getTargetMachine(), ArgLocs,
							*DAG.getContext());

	CCInfo.AnalyzeCallOperands(Outs, CC_TMS320C64X);
	bytes = CCInfo.getNextStackOffset();

	Chain = DAG.getCALLSEQ_START(Chain, DAG.getConstant(bytes,
						getPointerTy(), true));

	SmallVector<std::pair<unsigned int, SDValue>, 16> reg_args;
	SmallVector<SDValue, 16> stack_args;

	for (i = 0; i < ArgLocs.size(); ++i) {
		CCValAssign &va = ArgLocs[i];
		SDValue arg = Outs[i].Val;

		switch (va.getLocInfo()) {
		default: llvm_unreachable("Unknown arg loc");
		case CCValAssign::Full:
			break;
		case CCValAssign::SExt:
			arg = DAG.getNode(ISD::SIGN_EXTEND, dl,
						va.getLocVT(), arg);
			break;
		case CCValAssign::ZExt:
			arg = DAG.getNode(ISD::ZERO_EXTEND, dl,
						va.getLocVT(), arg);
			break;
		case CCValAssign::AExt:
			arg = DAG.getNode(ISD::ANY_EXTEND, dl,
						va.getLocVT(), arg);
			break;
		}

		if (va.isRegLoc()) {
			reg_args.push_back(std::make_pair(va.getLocReg(), arg));
		} else if (va.isMemLoc()) {
			SDValue stack_ptr = DAG.getCopyFromReg(Chain, dl,
								TMS320C64X::A15,
								getPointerTy());
			SDValue addr = DAG.getNode(ISD::ADD,
				dl, getPointerTy(), stack_ptr,
				DAG.getIntPtrConstant(va.getLocMemOffset()));

			SDValue store = DAG.getStore(Chain, dl, arg, addr,
							NULL, 0);
			stack_args.push_back(store);
		} else {
			llvm_unreachable("Invalid call argument location");
		}
	}

	// We now have two sets of register / stack locations to load stuff
	// into; apparently we can put the memory location ones into one big
	// chain, because they can happen independantly

	SDValue in_flag;
	if (!stack_args.empty()) {
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
					&stack_args[0], stack_args.size());
	}

	// This chains loading to specified registers sequentially
	for (i = 0; i < reg_args.size(); ++i) {
		Chain = DAG.getCopyToReg(Chain, dl, reg_args[i].first,
					reg_args[i].second, in_flag);
		in_flag = Chain.getValue(1);
	}

	// Following what MSP430 does, convert GlobalAddress nodes to
	// TargetGlobalAddresses.
	if (GlobalAddressSDNode *g = dyn_cast<GlobalAddressSDNode>(Callee)) {
		Callee = DAG.getTargetGlobalAddress(g->getGlobal(), MVT::i32);
	}  else if (ExternalSymbolSDNode *e =
				dyn_cast<ExternalSymbolSDNode>(Callee)) {
		Callee = DAG.getTargetExternalSymbol(e->getSymbol(), MVT::i32);
	}

	// Tie this all into a "Call"
	SmallVector<SDValue, 16> ops;
	ops.push_back(Chain);
	ops.push_back(Callee);

	for (i = 0; i < reg_args.size(); ++i) {
		ops.push_back(DAG.getRegister(reg_args[i].first,
				reg_args[i].second.getValueType()));
	}

	if (in_flag.getNode())
		ops.push_back(in_flag);

	SDVTList node_types = DAG.getVTList(MVT::Other, MVT::Flag);
	Chain = DAG.getNode(TMS320C64X::callp, dl, node_types, &ops[0],
							ops.size());
	in_flag = Chain.getValue(1);

	Chain = DAG.getCALLSEQ_END(Chain,
			DAG.getConstant(bytes, getPointerTy(), true),
			DAG.getConstant(0, getPointerTy(), true), in_flag);
	in_flag = Chain.getValue(1);

	return LowerCallResult(Chain, in_flag, CallConv, isVarArg, Ins, dl,
					DAG, InVals);
}

SDValue
TMS320C64XLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
				unsigned CallConv, bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals)
{
	unsigned int i;
	SmallVector<CCValAssign, 16> ret_locs;
	CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
			ret_locs, *DAG.getContext());
	CCInfo.AnalyzeCallResult(Ins, RetCC_TMS320C64X);

	for (i = 0; i < ret_locs.size(); ++i) {
		Chain = DAG.getCopyFromReg(Chain, dl, ret_locs[i].getLocReg(),
						ret_locs[i].getValVT(),
						InFlag).getValue(1);
		InFlag = Chain.getValue(2);
		InVals.push_back(Chain.getValue(0));
	}

	return Chain;
}

SDValue
TMS320C64XLowering::LowerOperation(SDValue op,  SelectionDAG &DAG)
{
	const TargetInstrInfo *TII = DAG.getTarget().getInstrInfo();
	const TargetInstrDesc &II = TII->get(op.getOpcode());
	switch (op.getOpcode()) {
	case ISD::GlobalAddress:	return LowerGlobalAddress(op, DAG);
	case ISD::RETURNADDR:		return LowerReturnAddr(op, DAG);
	default:
		llvm_unreachable(II.getName());
	}

	return op;
}

SDValue
TMS320C64XLowering::LowerGlobalAddress(SDValue op, SelectionDAG &DAG)
{

	const GlobalValue *GV = cast<GlobalAddressSDNode>(op)->getGlobal();
	int64_t offset = cast<GlobalAddressSDNode>(op)->getOffset();

	SDValue res = DAG.getTargetGlobalAddress(GV, getPointerTy(), offset);
	return DAG.getNode(TMS320C64X::mvkall, op.getDebugLoc(), getPointerTy(),
									res);
}

SDValue
TMS320C64XLowering::LowerReturnAddr(SDValue op, SelectionDAG &DAG)
{

	if (cast<ConstantSDNode>(op.getOperand(0))->getZExtValue() != 0)
		llvm_unreachable("LowerReturnAddr -> abnormal depth");

	// Right now, we always store ret addr -> slot 0.
	// Although it could be offset by something, not certain
	SDValue retaddr = DAG.getFrameIndex(0, getPointerTy());
	return DAG.getLoad(getPointerTy(), op.getDebugLoc(), DAG.getEntryNode(),
							retaddr, NULL, 0);
}
