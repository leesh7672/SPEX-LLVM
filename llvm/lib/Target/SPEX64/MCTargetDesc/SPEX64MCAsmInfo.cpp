//===-- SPEX64MCAsmInfo.cpp - SPEX64 MC asm info --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {
namespace {
class SPEX64MCAsmInfo : public MCAsmInfoELF {
public:
  explicit SPEX64MCAsmInfo(const Triple &, const MCTargetOptions &) {
    CodePointerSize = 8;
    CalleeSaveStackSlotSize = 8;
    IsLittleEndian = true;
    CommentString = ";";
    SupportsDebugInformation = true;
  }
};
} // namespace

MCAsmInfo *createSPEX64MCAsmInfo(const MCRegisterInfo &, const Triple &TT,
                                 const MCTargetOptions &Options) {
  return new SPEX64MCAsmInfo(TT, Options);
}
} // namespace llvm
