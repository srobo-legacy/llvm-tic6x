//==- TMS320C64XInstrInfo.h - TMS320C64X Instruction Information -*- C++ -*-==//
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

#ifndef LLVM_TARGET_TMS320C64X_INSTRINFO_H
#define LLVM_TARGET_TMS320C64X_INSTRINFO_H

#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XGenInstrNames.inc"
#include "TMS320C64XGenInstrInfo.inc"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/SelectionDAG.h"

namespace llvm {

class TMS320C64XTargetMachine;

namespace TMS320C64XII {
	enum {
		unit_d = 0,
		unit_s = 1,
		unit_l = 2,
		unit_m = 3,
		unit_1 = 0,
		unit_2 = 4, // Flag bit
		is_memaccess = 0x40,
		is_store = 0x80,
		mem_align_amt_mask = 0x300,
		mem_align_amt_shift = 8
	};
#define GET_UNIT(x) ((x) & 3)
#define GET_SIDE(x) ((x) & 4)
#define GET_DELAY_SLOTS(x) (((x) >> 3) & 0x7)
}

class TMS320C64XInstrInfo : public TargetInstrInfoImpl {
	const TMS320C64XRegisterInfo RI;
	TMS320C64XTargetMachine &TM;
public:
	explicit TMS320C64XInstrInfo(TMS320C64XTargetMachine &TM);

	virtual const TargetRegisterInfo &getRegisterInfo() const { return RI; }

	virtual bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
				MachineBasicBlock::iterator MBBI,
				const std::vector<CalleeSavedInfo> &CSI) const;

	virtual bool copyRegToReg(MachineBasicBlock &MBB,
				MachineBasicBlock::iterator I,
				unsigned desg_reg, unsigned src_reg,
				const TargetRegisterClass *dst_rc,
				const TargetRegisterClass *src_rc) const;
	virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
				MachineBasicBlock::iterator I,
				unsigned src_reg, bool is_kill, int FI,
				const TargetRegisterClass *rc) const;
	virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
				MachineBasicBlock::iterator MI,
				unsigned dst_reg, int frame_idx,
				const TargetRegisterClass *RC) const;
	virtual bool AnalyzeBranch(MachineBasicBlock &MBB,
				MachineBasicBlock *&TBB,
				MachineBasicBlock *&FBB,
				SmallVectorImpl<MachineOperand> &Cond,
				bool AllowModify = false) const;
	virtual unsigned InsertBranch(MachineBasicBlock &MBB,
				MachineBasicBlock *TBB,
				MachineBasicBlock *FBB,
				const SmallVectorImpl<MachineOperand> &Cond)
				const;
	virtual unsigned RemoveBranch(MachineBasicBlock &MBB) const;
};

inline const MachineInstrBuilder &addDefaultPred(const MachineInstrBuilder &MIB)
{

        return MIB.addImm(-1).addReg(TMS320C64X::NoRegister);
}

inline const TargetRegisterClass *
findRegisterSide(unsigned reg, const MachineFunction *MF)
{       
        int j;
        TargetRegisterClass *c;

        TargetRegisterClass::iterator i =
                TMS320C64X::ARegsRegisterClass->allocation_order_begin(*MF);
        c = TMS320C64X::BRegsRegisterClass;
        // Hackity: don't use allocation_order_end, because it won't
        // match instructions that use reserved registers, and they'll
        // incorrectly get marked as being on the other data path side.
        // So instead, we know that there's 32 of them in the A reg
        // class, just loop through all of them
        for (j = 0; j < 32; j++) {
                if ((*i) == reg) {
                        c = TMS320C64X::ARegsRegisterClass;
                        break;
                }
                i++;
        }

        return c;
}

// Some utility functions for checking that constants fit in a bit field in
// an instruction. The sconst checking routines will verify that a signed
// number will be representable withing the bitfield. The uconst checks change
// the number to be absolute, then sees whether it fits in the bit field.
// Finally there's a check to see whether a constant sdnode is positive or not

inline bool
check_sconst_fits(long int num, int bits)
{
	long int maxval;

	maxval = 1 << (bits - 1);
	if (num < maxval && num >= (-maxval))
		return true;

	return false;
}

inline bool
check_uconst_fits(unsigned long int num, int bits)
{
	unsigned long maxval;

	maxval = 1 << bits;
	if (num < maxval)
		return true;

	return false;
}

inline bool
Predicate_sconst_n(SDNode *in, int bits)
{
	ConstantSDNode *N;
	int val;

	N = cast<ConstantSDNode>(in);
	val = N->getSExtValue();
	return check_sconst_fits(val, bits);
}

inline bool
Predicate_uconst_n(SDNode *in, int bits)
{
	ConstantSDNode *N;
	unsigned int val;

	N = cast<ConstantSDNode>(in);
	val = N->getSExtValue();
	val = abs(val);
	return check_uconst_fits(val, bits);
}

inline bool
Predicate_const_is_positive(SDNode *in)
{
	ConstantSDNode *N;
	unsigned long val;

	N = cast<ConstantSDNode>(in);
	val = N->getSExtValue();
	return (val >= 0);
}

} // llvm

#endif // LLVM_TARGET_TMS320C64X_INSTRINFO_H
