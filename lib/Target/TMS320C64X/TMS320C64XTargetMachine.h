//==- TMS320C64XTargetMachine.h - Define TargetMachine for tic64x *- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TARGET_TMS320C64X_TARGETMACHINE_H
#define LLVM_TARGET_TMS320C64X_TARGETMACHINE_H

#include "TMS320C64XSubtarget.h"
#include "TMS320C64XLowering.h"
#include "TMS320C64XInstrInfo.h"

#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameInfo.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class TMS320C64XTargetMachine : public LLVMTargetMachine {
	TMS320C64XSubtarget		Subtarget;
	const TargetData		DataLayout;
	TMS320C64XInstrInfo		InstrInfo;
	TMS320C64XLowering		TLInfo;
	/* Can't see another backend that subclasses this */
	TargetFrameInfo	FrameInfo;

public:
	TMS320C64XTargetMachine(const Target &T, const std::string &TT,
						const std::string &FS);

	virtual const TargetFrameInfo *getFrameInfo()
			const { return &FrameInfo; }
	virtual const TMS320C64XInstrInfo *getInstrInfo()
			const { return &InstrInfo; }
	virtual const TargetData *getTargetData()
			const { return &DataLayout;}
	virtual const TMS320C64XSubtarget *getSubtargetImpl()
			const { return &Subtarget; }
	virtual const TargetRegisterInfo *getRegisterInfo()
			const { return &InstrInfo.getRegisterInfo(); }
	virtual TMS320C64XLowering *getTargetLowering()
		const { return const_cast<TMS320C64XLowering*>(&TLInfo); }
	virtual bool addInstSelector(PassManagerBase &PM,
					CodeGenOpt::Level OptLevel);

	virtual bool getEnableTailMergeDefault() const { return false; }
		// Don't wish to overcomplicate things right now
};

} // namespace llvm

#endif // LLVM_TARGET_TMS320C64X_TARGETMACHINE_H
