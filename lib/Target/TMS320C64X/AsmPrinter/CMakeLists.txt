include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMTMS320C64XAsmPrinter
  TMS320C64XAsmPrinter.cpp
  )
add_dependencies(LLVMTMS302C64XAsmPrinter TMS320C64X)
