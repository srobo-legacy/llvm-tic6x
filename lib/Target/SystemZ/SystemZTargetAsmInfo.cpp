//===-- SystemZTargetAsmInfo.cpp - SystemZ asm properties -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the SystemZTargetAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "SystemZTargetAsmInfo.h"
#include "SystemZTargetMachine.h"

using namespace llvm;

SystemZTargetAsmInfo::SystemZTargetAsmInfo(const SystemZTargetMachine &TM)
  : ELFTargetAsmInfo(TM) {
  AlignmentIsInBytes = false;
}