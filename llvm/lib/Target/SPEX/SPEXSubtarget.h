//===-- SPEXSubtarget.h - SPEX subtarget declarations --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXSUBTARGET_H
#define LLVM_LIB_TARGET_SPEX_SPEXSUBTARGET_H

#include "SPEXFrameLowering.h"
#include "SPEXSelectionDAGInfo.h"
#include "SPEXISelLowering.h"
#include "SPEXInstrInfo.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/TargetParser/Triple.h"

#define GET_SUBTARGETINFO_HEADER
#include "SPEXGenSubtargetInfo.inc"

namespace llvm {

class SPEXTargetMachine;

class SPEXSubtarget : public SPEXGenSubtargetInfo {
public:
  enum Lane : unsigned { Lane0 = 0, Lane1 = 1, Lane2 = 2, Lane3 = 3 };

private:
  Lane CurLane = Lane0;

  SPEXInstrInfo InstrInfo;
  SPEXFrameLowering FrameLowering;
  SPEXTargetLowering TLInfo;
  SPEXSelectionDAGInfo TSInfo;

public:
  SPEXSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                SPEXTargetMachine &TM);
  ~SPEXSubtarget() override;

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  void setLaneID(unsigned V) { CurLane = static_cast<Lane>(V & 3u); }
  Lane getLane() const { return CurLane; }

  const TargetInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetRegisterInfo *getRegisterInfo() const override;
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const TargetLowering *getTargetLowering() const override { return &TLInfo; }
};

} // namespace llvm
#endif
