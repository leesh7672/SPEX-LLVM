//===-- SPEX64TargetInfo.cpp - SPEX64 target info --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheSPEX64Target() {
  static Target TheSPEX64Target;
  return TheSPEX64Target;
}

extern "C" void LLVMInitializeSPEX64TargetInfo() {
  RegisterTarget<Triple::spex64, /*HasJIT=*/false> X(
      getTheSPEX64Target(), "spex64", "SPEX64", "SPEX64");
}
