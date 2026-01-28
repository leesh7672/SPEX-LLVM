//===-- SPEXSubtarget.cpp - SPEX subtarget implementation --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXSubtarget.h"

using namespace llvm;

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SPEXGenSubtargetInfo.inc"

SPEXSubtarget::SPEXSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                 SPEXTargetMachine &TM)
    : SPEXGenSubtargetInfo(TT, CPU, CPU, FS), InstrInfo(*this),
      FrameLowering(), TLInfo(TM, *this) {
  ParseSubtargetFeatures(CPU, CPU, FS);
}

SPEXSubtarget::~SPEXSubtarget() = default;

const TargetRegisterInfo *SPEXSubtarget::getRegisterInfo() const {
  return &InstrInfo.getRegisterInfo();
}
