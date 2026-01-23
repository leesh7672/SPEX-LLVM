//===-- SPEX64Subtarget.cpp - SPEX64 subtarget implementation --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64Subtarget.h"

using namespace llvm;

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SPEX64GenSubtargetInfo.inc"

SPEX64Subtarget::SPEX64Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                 SPEX64TargetMachine &TM)
    : SPEX64GenSubtargetInfo(TT, CPU, CPU, FS), InstrInfo(*this),
      FrameLowering(), TLInfo(TM, *this) {
  ParseSubtargetFeatures(CPU, CPU, FS);
}

SPEX64Subtarget::~SPEX64Subtarget() = default;

const TargetRegisterInfo *SPEX64Subtarget::getRegisterInfo() const {
  return &InstrInfo.getRegisterInfo();
}
