##===- lib/Target/TMS320C64X/Makefile ----------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
# what student robotics decides to do with this code
# Derived from MSP430 implementation, see LLVM's LICENSE.TXT
# 
##===----------------------------------------------------------------------===##
LEVEL = ../../..
LIBRARYNAME = LLVMTMS320C64XCodeGen
TARGET = TMS320C64X

# Make sure that tblgen is run, first thing.
BUILT_SOURCES =	TMS320C64XGenRegisterInfo.h.inc \
		TMS320C64XGenRegisterNames.inc \
		TMS320C64XGenRegisterInfo.inc \
		TMS320C64XGenInstrNames.inc \
		TMS320C64XGenInstrInfo.inc \
		TMS320C64XTargetMachine.cpp TMS320C64XSubtarget.cpp \
		TMS320C64XInstrInfo.cpp TMS320C64XRegisterInfo.cpp \
		TMS320C64XLowering.cpp

DIRS = TargetInfo

include $(LEVEL)/Makefile.common

