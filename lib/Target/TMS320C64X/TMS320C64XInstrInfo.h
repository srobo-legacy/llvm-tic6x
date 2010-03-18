//==- TMS320C64XInstrInfo.h - TMS320C64X Instruction Information -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from MSP430 implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_TMS320C64X_INSTRINFO_H
#define LLVM_TARGET_TMS320C64X_INSTRINFO_H

#include "TMS320C64XRegisterInfo.h"
#include "TMS320C64XGenInstrNames.inc"
#include "TMS320C64XGenInstrInfo.inc"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

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
		is_store = 0x80
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


} // llvm

#endif // LLVM_TARGET_TMS320C64X_INSTRINFO_H
