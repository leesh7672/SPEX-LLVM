//===-- SPEX64Subtarget.h - SPEX64 subtarget declarations --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64SUBTARGET_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64SUBTARGET_H

#include "SPEX64FrameLowering.h"
#include "SPEX64ISelLowering.h"
#include "SPEX64InstrInfo.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/TargetParser/Triple.h"

#define GET_SUBTARGETINFO_HEADER
#include "SPEX64GenSubtargetInfo.inc"

namespace llvm {

class SPEX64TargetMachine;

class SPEX64Subtarget : public SPEX64GenSubtargetInfo {
public:
  enum Lane : unsigned { Lane0 = 0, Lane1 = 1, Lane2 = 2, Lane3 = 3 };

private:
  Lane CurLane = Lane0;

  SPEX64InstrInfo InstrInfo;
  SPEX64FrameLowering FrameLowering;
  SPEX64TargetLowering TLInfo;

public:
  SPEX64Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                  SPEX64TargetMachine &TM);
  ~SPEX64Subtarget() override;

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  void setLaneID(unsigned V) { CurLane = static_cast<Lane>(V & 3u); }
  Lane getLane() const { return CurLane; }

  const TargetInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetRegisterInfo *getRegisterInfo() const override;
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const TargetLowering *getTargetLowering() const override { return &TLInfo; }
};

} // namespace llvm
#endif
