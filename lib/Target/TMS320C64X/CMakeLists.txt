set(LLVM_TARGET_DEFINITIONS TMS320C64X.td)

tablegen(TMS320C64XGenRegisterInfo.h.inc -gen-register-desc-header)
tablegen(TMS320C64XGenRegisterNames.inc -gen-register-enums)
tablegen(TMS320C64XGenRegisterInfo.inc -gen-register-desc)
tablegen(TMS320C64XGenInstrNames.inc -gen-instr-enums)
tablegen(TMS320C64XGenInstrInfo.inc -gen-instr-desc)
//tablegen(TMS320C64XGenAsmWriter.inc -gen-asm-writer)

add_llvm_target(TMS320C64X
  TMS320C64XInstrInfo.cpp
  TMS320C64XLowering.cpp
  TMS320C64XRegisterInfo.cpp
  TMS320C64XSubtarget.cpp
  TMS320C64XTargetAsmInfo.cpp
  TMS320C64XTargetMachine.cpp
  )

target_link_libraries (LLVMTMS320C64X LLVMSelectionDAG)
