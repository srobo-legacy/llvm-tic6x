//===-- TMS320C64XLowering.cpp - TMS320C64X DAG Lowering Implementation  --===//
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
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CallingConv.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/ADT/VectorExtras.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

namespace llvm {
#include "TMS320C64XGenCallingConv.inc"
}

TMS320C64XLowering::TMS320C64XLowering(TMS320C64XTargetMachine &tm) :
	TargetLowering(tm, new TargetLoweringObjectFileCOFF()), TM(tm)
{

	addRegisterClass(MVT::i32, TMS320C64X::GPRegsRegisterClass);
	addRegisterClass(MVT::i32, TMS320C64X::ARegsRegisterClass);
	addRegisterClass(MVT::i32, TMS320C64X::BRegsRegisterClass);
	addRegisterClass(MVT::i32, TMS320C64X::PredRegsRegisterClass);

	setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);
	setLoadExtAction(ISD::EXTLOAD, MVT::i1, Custom);
	setLoadExtAction(ISD::EXTLOAD, MVT::i8, Custom);
	setLoadExtAction(ISD::EXTLOAD, MVT::i16, Custom);
	setLoadExtAction(ISD::EXTLOAD, MVT::i32, Custom);
	// All other loads have sx and zx support

	// No 32 bit load insn
	setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
	setOperationAction(ISD::JumpTable, MVT::i32, Custom);

	// No in-reg sx
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
	setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

	// No divide anything
	setOperationAction(ISD::UDIV, MVT::i32, Expand);
	setOperationAction(ISD::SDIV, MVT::i32, Expand);
	setOperationAction(ISD::UREM, MVT::i32, Expand);
	setOperationAction(ISD::SREM, MVT::i32, Expand);
	setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
	setOperationAction(ISD::SDIVREM, MVT::i32, Expand);

	// We can generate two conditional instructions for select, not so
	// easy for select_cc
	setOperationAction(ISD::SELECT, MVT::i32, Custom);
	setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
	// Manually beat condition code setting into cmps
	setOperationAction(ISD::SETCC, MVT::i32, Custom);
	// We can emulate br_cc, maybe not brcond, do what works
	setOperationAction(ISD::BRCOND, MVT::i32, Expand);
	setOperationAction(ISD::BR_CC, MVT::i32, Custom);
	setOperationAction(ISD::BR_JT, MVT::Other, Expand);

	// Probably is a membarrier, but I'm not aware of it right now
	setOperationAction(ISD::MEMBARRIER, MVT::Other, Expand);

	// Should also inject other invalid operations here
	// The following might be supported, but I can't be bothered or don't
	// have enough time to work on
	setOperationAction(ISD::MULHS, MVT::i32, Expand);
	setOperationAction(ISD::MULHU, MVT::i32, Expand);
	setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
	setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
	setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
	setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
	setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);

	// There's no carry support, we're supposed to use 40 bit integers
	// instead. Let llvm generate workarounds instead.
	setOperationAction(ISD::ADDC, MVT::i32, Expand);
	setOperationAction(ISD::SUBC, MVT::i32, Expand);
	setOperationAction(ISD::ADDE, MVT::i32, Expand);
	setOperationAction(ISD::SUBE, MVT::i32, Expand);

	// VACOPY and VAEND apparently have sane defaults, however
	// VASTART and VAARG can't be expanded
	setOperationAction(ISD::VASTART, MVT::Other, Custom);
	setOperationAction(ISD::VAARG, MVT::Other, Custom);
	setOperationAction(ISD::VACOPY, MVT::Other, Expand);
	setOperationAction(ISD::VAEND, MVT::Other, Expand);

	setStackPointerRegisterToSaveRestore(TMS320C64X::A15);

	computeRegisterProperties();
	return;
}

TMS320C64XLowering::~TMS320C64XLowering()
{

	// _nothing_
	return;
}

const char *
TMS320C64XLowering::getTargetNodeName(unsigned op) const
{

	switch (op) {
	default: return NULL;
	case TMSISD::BRCOND:		return "TMSISD::BRCOND";
	case TMSISD::CALL:		return "TMSISD::CALL";
	case TMSISD::CALL_RET_LABEL:	return "TMSISD::CALL_RET_LABEL";
	case TMSISD::CALL_RET_LABEL_OPERAND:
					return "TMSISD::CALL_RET_LABEL_OPERAND";
	case TMSISD::CMPEQ:		return "TMSISD::CMPEQ";
	case TMSISD::CMPNE:		return "TMSISD::CMPNE";
	case TMSISD::CMPGT:		return "TMSISD::CMPGT";
	case TMSISD::CMPGTU:		return "TMSISD::CMPGTU";
	case TMSISD::CMPLT:		return "TMSISD::CMPLT";
	case TMSISD::CMPLTU:		return "TMSISD::CMPLTU";
	case TMSISD::RET_FLAG:		return "TMSISD::RET_FLAG";
	case TMSISD::WRAPPER:		return "TMSISD::WRAPPER";
	}
}

unsigned
TMS320C64XLowering::getFunctionAlignment(Function const*) const
{

	return 5; // 32 bytes; instruction packet
}

using namespace TMS320C64X;
SDValue
TMS320C64XLowering::LowerFormalArguments(SDValue Chain,
				CallingConv::ID CallConv,bool isVarArg,
				const SmallVectorImpl<ISD::InputArg> &Ins,
				DebugLoc dl, SelectionDAG &DAG,
				SmallVectorImpl<SDValue> &InVals)
{
	SmallVector<CCValAssign, 16> ArgLocs;
	static const unsigned int arg_regs[] =
		{ A4, B4, A6, B6, A8, B8, A10, B10, A12, B12 };
	unsigned int i, arg_idx, reg, stack_offset, last_fixed_arg;

	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	MachineRegisterInfo &RegInfo = MF.getRegInfo();

	arg_idx = 0;
	stack_offset = 0;

	CCState CCInfo(CallConv, isVarArg, getTargetMachine(), ArgLocs,
			*DAG.getContext());
	CCInfo.AnalyzeFormalArguments(Ins, CC_TMS320C64X);

	// TI calling convention dictates that the argument preceeding the
	// variable list of arguments must also be placed on the stack.
	// This doesn't cause a problem as C dictates there must always be one
	// non-var argument. But in any case, we need to break out of the
	// register-munging loop and ensure the last fixed argument goes to
	// the stack.
	last_fixed_arg = ArgLocs.size() - 1;

	// Ditch location allocation of arguments and do our own thing - only
	// way to make varargs work correctly
	for (i = 0; i < ArgLocs.size(); ++i) {
		CCValAssign &VA = ArgLocs[i];
		EVT ObjectVT = VA.getValVT();
		switch (ObjectVT.getSimpleVT().SimpleTy) {
		default: llvm_unreachable("Unhandled argument type");
		case MVT::i1:
		case MVT::i8:
		case MVT::i16:
		case MVT::i32:
			if (!Ins[i].Used && (!isVarArg || i != last_fixed_arg)){
				if (arg_idx < 10) arg_idx++;
				InVals.push_back(DAG.getUNDEF(ObjectVT));
			} else if (arg_idx < 10 &&
					(!isVarArg || i != last_fixed_arg)) {
				reg = RegInfo.createVirtualRegister(
						&GPRegsRegClass);
				MF.getRegInfo().addLiveIn(arg_regs[arg_idx++],
									reg);
				SDValue Arg = DAG.getCopyFromReg(Chain, dl,
							reg, MVT::i32);
				if (ObjectVT != MVT::i32) {
					Arg = DAG.getNode(ISD::AssertSext,
						dl, MVT::i32, Arg,
						DAG.getValueType(ObjectVT));
					Arg = DAG.getNode(ISD::TRUNCATE, dl,
						ObjectVT, Arg);
				}
				InVals.push_back(Arg);
			} else {
				// XXX - i64?
				int frame_idx = MFI->CreateFixedObject(4,
						stack_offset, true, false);
				SDValue FIPtr = DAG.getFrameIndex(frame_idx,
								MVT::i32);
				SDValue load;
				if (ObjectVT == MVT::i32) {
					// XXX - Non temporal? Eh?
					load = DAG.getLoad(MVT::i32, dl, Chain,
							FIPtr, NULL, 0, false,
							false, 4);
				} else {
					// XXX - work out alignment
					load = DAG.getExtLoad(ISD::SEXTLOAD,
							dl, MVT::i32,  Chain,
							FIPtr, NULL, 0,
							ObjectVT, false, false,
							4);
					load = DAG.getNode(ISD::TRUNCATE, dl,
							ObjectVT, load);
				}
				InVals.push_back(load);

				stack_offset += 4;
			}
		} // switch
	} // argloop

	return Chain;
}

SDValue
TMS320C64XLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
				bool isVarArg,
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

		Flag = Chain.getValue(1);
	}

	if (Flag.getNode())
		return DAG.getNode(TMSISD::RET_FLAG, dl, MVT::Other, Chain,
									Flag);
	return DAG.getNode(TMSISD::RET_FLAG, dl, MVT::Other, Chain);
}

SDValue
TMS320C64XLowering::LowerCall(SDValue Chain, SDValue Callee, CallingConv::ID
			CallConv, bool isVarArg, bool &isTailCall,
			const SmallVectorImpl<ISD::OutputArg> &Outs,
			const SmallVectorImpl<ISD::InputArg> &Ins,
			DebugLoc dl, SelectionDAG &DAG,
			SmallVectorImpl<SDValue> &InVals)
{
	SmallVector<CCValAssign, 16> ArgLocs;
	static const unsigned int reg_arg_nums[] =
		{ A4, B4, A6, B6, A8, B8, A10, B10, A12, B12 };
	int bytes, stacksize;
	unsigned int i, retaddr, arg_idx, fixed_args;
	bool is_icall = false;;

	retaddr = 0;
	arg_idx = 0;
	bytes = 0;
	fixed_args = 0;

	isTailCall = false;

	CCState CCInfo(CallConv, isVarArg, getTargetMachine(), ArgLocs,
							*DAG.getContext());
	CCInfo.AnalyzeCallOperands(Outs, CC_TMS320C64X);

	for (i = 0; i < Outs.size(); i++) {
		if (Outs[i].IsFixed)
			fixed_args++;
	}

	// Make our own stack and register decisions; however keep CCInfos
	// thoughts on sign extension, as they're handy.
	// Start out by guessing how much stack space we need
	if (!isVarArg) {
		if (ArgLocs.size() > 10) {
			stacksize = (ArgLocs.size() - 10) * 4; // XXX - i64?
		} else {
			stacksize = 0;
		}
	} else {
		stacksize = (ArgLocs.size() - fixed_args + 1) * 4;
	}

	Chain = DAG.getCALLSEQ_START(Chain, DAG.getTargetConstant(stacksize,
						MVT::i32));

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

		if (arg_idx < 10 && (!isVarArg || i < fixed_args - 1)) {
			// Additional check to ensure last fixed param and all
			// variable params go on stack, if we're vararging
			reg_args.push_back(std::make_pair(reg_arg_nums[arg_idx],
									arg));
			arg_idx++;
		} else {
			bytes += 4; /* Offset from SP at call */

			SDValue stack_ptr = DAG.getCopyFromReg(Chain, dl,
								TMS320C64X::B15,
								getPointerTy());
			SDValue addr = DAG.getNode(ISD::ADD,
				dl, getPointerTy(), stack_ptr,
				DAG.getIntPtrConstant(bytes));

			SDValue store = DAG.getStore(Chain, dl, arg, addr,
							NULL, 0, false, false,
							4);
			stack_args.push_back(store);
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

	if(Callee.getOpcode() != ISD::TargetGlobalAddress &&
			Callee.getOpcode() != ISD::GlobalAddress &&
			Callee.getOpcode() != ISD::TargetExternalSymbol &&
			Callee.getOpcode() != ISD::ExternalSymbol) {
		// This an indirect call.
		// Unfortunately there's no direct call instruction for this...
		// we have to emulate it. That involves sticking a label
		// directly after the call/branch, and loading its address into
		// b3 directly beforehand. Ugh.

		// Generate a return address
		MachineModuleInfo *MMI = DAG.getMachineModuleInfo();
		retaddr = MMI->NextLabelID();
		in_flag = Chain.getValue(0);

		SDVTList NodeTys = DAG.getVTList(MVT::i32, MVT::Flag);
		Chain = DAG.getNode(TMSISD::CALL_RET_LABEL_OPERAND, dl, NodeTys,
				Chain, DAG.getConstant(retaddr, MVT::i32),
				in_flag);
		in_flag = Chain.getValue(1);
		Chain = DAG.getCopyToReg(Chain, dl, TMS320C64X::B3,
							Chain, in_flag);
		in_flag = Chain.getValue(1);

		is_icall = true;
	}

	if (in_flag.getNode())
		ops.push_back(in_flag);

	SDVTList node_types = DAG.getVTList(MVT::Other, MVT::Flag);
	Chain = DAG.getNode(TMSISD::CALL, dl, node_types, &ops[0], ops.size());
	in_flag = Chain.getValue(1);

	if (is_icall) {
		// pump in a label directly after that call insn
		SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Flag);
		Chain = DAG.getNode(TMSISD::CALL_RET_LABEL, dl, NodeTys,
				Chain, DAG.getConstant(retaddr, MVT::i32),
				in_flag);
		in_flag = Chain.getValue(1);
	}

	Chain = DAG.getCALLSEQ_END(Chain,
			DAG.getTargetConstant(stacksize, MVT::i32),
			DAG.getTargetConstant(0, MVT::i32),
			in_flag);
	in_flag = Chain.getValue(1);

	return LowerCallResult(Chain, in_flag, CallConv, isVarArg, Ins, dl,
					DAG, InVals);
}

SDValue
TMS320C64XLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
				CallingConv::ID CallConv, bool isVarArg,
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
	switch (op.getOpcode()) {
	case ISD::GlobalAddress:	return LowerGlobalAddress(op, DAG);
	case ISD::JumpTable:		return LowerJumpTable(op, DAG);
	case ISD::RETURNADDR:		return LowerReturnAddr(op, DAG);
	case ISD::BR_CC:		return LowerBRCC(op, DAG);
	case ISD::SETCC:		return LowerSETCC(op, DAG);
	// We only ever get custom loads when it's an extload
	case ISD::LOAD:			return LowerExtLoad(op, DAG);
	case ISD::SELECT:		return LowerSelect(op, DAG);
	case ISD::VASTART:		return LowerVASTART(op, DAG);
	case ISD::VAARG:		return LowerVAARG(op, DAG);
	default:
		llvm_unreachable(op.getNode()->getOperationName().c_str());
	}

	return op;
}

SDValue
TMS320C64XLowering::LowerGlobalAddress(SDValue op, SelectionDAG &DAG)
{

	const GlobalValue *GV = cast<GlobalAddressSDNode>(op)->getGlobal();
	int64_t offset = cast<GlobalAddressSDNode>(op)->getOffset();

	SDValue res = DAG.getTargetGlobalAddress(GV, getPointerTy(), offset);
	return DAG.getNode(TMSISD::WRAPPER, op.getDebugLoc(), getPointerTy(),
									res);
}

SDValue
TMS320C64XLowering::LowerJumpTable(SDValue op, SelectionDAG &DAG)
{

	const JumpTableSDNode *j = cast<JumpTableSDNode>(op);

	SDValue res = DAG.getTargetJumpTable(j->getIndex(), getPointerTy(), 0);
	return DAG.getNode(TMSISD::WRAPPER, op.getDebugLoc(), getPointerTy(),
									res);
}

SDValue
TMS320C64XLowering::LowerExtLoad(SDValue op, SelectionDAG &DAG)
{

	const LoadSDNode *l = cast<LoadSDNode>(op);
	SDVTList list = l->getVTList();

	return DAG.getExtLoad(ISD::SEXTLOAD, op.getDebugLoc(), list.VTs[0],
			l->getOperand(0), l->getOperand(1), l->getSrcValue(),
			l->getSrcValueOffset(), l->getMemoryVT(),
			l->isVolatile(), false, l->getAlignment());
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
					retaddr, NULL, 0, false, false, 4);
}

SDValue
TMS320C64XLowering::LowerBRCC(SDValue op, SelectionDAG &DAG)
{
	int pred = 1;
	ISD::CondCode cc = cast<CondCodeSDNode>(op.getOperand(1))->get();

	// Can't do setne: instead invert predicate
	if (cc == ISD::SETNE || cc== ISD::SETUNE) {
		cc = ISD::SETEQ;
		pred = 0;
	}

	SDValue Chain = DAG.getSetCC(op.getDebugLoc(), MVT::i32,
				op.getOperand(2), op.getOperand(3), cc);
	if (Chain.getNode()->getOpcode() != ISD::Constant)
		Chain = LowerSETCC(Chain, DAG);

	// Generate our own brcond form, operands BB, const/reg for predicate
	Chain = DAG.getNode(TMSISD::BRCOND, op.getDebugLoc(), MVT::Other,
		op.getOperand(0), op.getOperand(4),
		DAG.getTargetConstant(pred, MVT::i32), Chain);

	return Chain;
}

SDValue
TMS320C64XLowering::LowerSETCC(SDValue op, SelectionDAG &DAG)
{
	unsigned int opcode;
	SDValue setop, tmp;
	SDValue lhs = op.getOperand(0);
	SDValue rhs = op.getOperand(1);
	ISD::CondCode cc = cast<CondCodeSDNode>(op.getOperand(2))->get();

#define SWAP() {		\
	tmp = lhs;		\
	lhs = rhs;		\
	rhs = tmp;		\
}

	switch (cc) {
	case ISD::SETFALSE:
	case ISD::SETTRUE:
	case ISD::SETFALSE2:
	case ISD::SETTRUE2:
	case ISD::SETCC_INVALID:
	case ISD::SETOEQ:
	case ISD::SETOGT:
	case ISD::SETOGE:
	case ISD::SETOLT:
	case ISD::SETOLE:
	case ISD::SETONE:
	case ISD::SETO:
	case ISD::SETUO:
	default:
		llvm_unreachable("Unsupported condcode");
	case ISD::SETEQ:
	case ISD::SETUEQ:
		opcode = TMSISD::CMPEQ;
		break;
	case ISD::SETUGT:
		opcode = TMSISD::CMPGTU;
		break;
	case ISD::SETUGE:
		opcode = TMSISD::CMPLTU;
		SWAP();
		break;
	case ISD::SETULT:
		opcode = TMSISD::CMPLTU;
		break;
	case ISD::SETULE:
		opcode = TMSISD::CMPGTU;
		SWAP();
		break;
	case ISD::SETUNE:
		opcode = TMSISD::CMPNE;
		break;
	case ISD::SETGT:
		opcode = TMSISD::CMPGT;
		break;
	case ISD::SETGE:
		opcode = TMSISD::CMPLT;
		SWAP();
		break;
	case ISD::SETLT:
		opcode = TMSISD::CMPLT;
		break;
	case ISD::SETLE:
		opcode = TMSISD::CMPGT;
		SWAP();
		break;
	case ISD::SETNE:
		opcode = TMSISD::CMPNE;
		break;
	}

	return  DAG.getNode(opcode, op.getDebugLoc(), MVT::i32, lhs, rhs);
}

SDValue
TMS320C64XLowering::LowerSelect(SDValue op, SelectionDAG &DAG)
{
	SDValue ops[6];

	// Operand 1 is true/false, selects operand 2 or 3 respectively
	// We'll generate this with two conditional move instructions - moving
	// the true/false result to the same register. In theory these could
	// be scheduled in the same insn packet (given that only one will
	// execute out of the pair, due to the conditional)

	ops[0] = op.getOperand(1);
	ops[1] = op.getOperand(2);
	ops[2] = DAG.getTargetConstant(0, MVT::i32);
	ops[3] = op.getOperand(0);
	ops[4] = DAG.getTargetConstant(1, MVT::i32);
	ops[5] = op.getOperand(0);
	return DAG.getNode(TMSISD::SELECT, op.getDebugLoc(), MVT::i32, ops, 6);
}

SDValue
TMS320C64XLowering::LowerVASTART(SDValue op, SelectionDAG &DAG)
{
	int num_normal_params, stackgap;

	num_normal_params = DAG.getMachineFunction().getFunction()->
					getFunctionType()->getNumParams();
	// As referenced elsewhere, TI specify the last fixed argument has to
	// go on the stack. Also, any other overflow.
	if (num_normal_params <= 10) {
		stackgap = 4;
	} else {
		stackgap = (num_normal_params - 10) * 4;
	}

	SDValue Chain = DAG.getNode(ISD::ADD, op.getDebugLoc(), MVT::i32,
				DAG.getRegister(TMS320C64X::B15, MVT::i32),
				DAG.getConstant(stackgap, MVT::i32));
	const Value *SV = cast<SrcValueSDNode>(op.getOperand(2))->getValue();
	return DAG.getStore(op.getOperand(0), op.getDebugLoc(), Chain,
				op.getOperand(1), SV, 0, false, false, 4);
}

SDValue
TMS320C64XLowering::LowerVAARG(SDValue op, SelectionDAG &DAG)
{

	// Largely copy sparc
	SDNode *n = op.getNode();
	EVT vt = n->getValueType(0);
	SDValue chain = n->getOperand(0);
	SDValue valoc = n->getOperand(1);
	const Value *SV = cast<SrcValueSDNode>(n->getOperand(2))->getValue();

	// Load point to vaarg list
	SDValue loadptr = DAG.getLoad(MVT::i32, op.getDebugLoc(), chain,
					valoc, SV, 0, false, false, 4);
	// Calculate address of next vaarg
	SDValue newptr = DAG.getNode(ISD::ADD, op.getDebugLoc(), MVT::i32,
			chain, DAG.getConstant(vt.getSizeInBits()/8, MVT::i32));
	// Store that back to wherever we're storing the vaarg list
	chain = DAG.getStore(loadptr.getValue(1), op.getDebugLoc(),
				newptr, valoc, SV, 0, false, false, 4);

	// Actually load the argument
	return DAG.getLoad(vt, op.getDebugLoc(), chain, loadptr, NULL, 0,
					false, false, 4);
}
