//===-- SPEXTargetInfo.cpp - SPEX target info --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheSPEXTarget() {
  static Target TheSPEXTarget;
  return TheSPEXTarget;
}

extern "C" void LLVMInitializeSPEXTargetInfo() {
  RegisterTarget<Triple::spex64, /*HasJIT=*/false> X(
      getTheSPEXTarget(), "spex64", "SPEX", "SPEX");
}
