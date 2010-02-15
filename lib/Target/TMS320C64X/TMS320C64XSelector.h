//=- TMS320C64XSelector.h - Instruction selector header for tic64x *- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
//
//===----------------------------------------------------------------------===//

#include "TMS320C64XTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

namespace llvm {
class TMS320C64XInstSelectorPass : public SelectionDAGISel {
public:
	explicit TMS320C64XInstSelectorPass(TMS320C64XTargetMachine &TM);

	void InstructionSelect();
	const char *getPassName() const {
		return "TMS320C64X Instruction Selection";
	}
};
}
