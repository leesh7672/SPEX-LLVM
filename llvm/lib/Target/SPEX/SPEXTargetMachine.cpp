//===-- SPEXTargetMachine.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXTargetMachine.h"
#include "TargetInfo/SPEXTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

extern "C" void LLVMInitializeSPEXTargetMC();
extern "C" void LLVMInitializeSPEXAsmPrinter();

extern "C" void LLVMInitializeSPEXTarget() {
  LLVMInitializeSPEXTargetMC();
  LLVMInitializeSPEXAsmPrinter();
  RegisterTargetMachine<SPEXTargetMachine> X(getTheSPEXTarget());
}
