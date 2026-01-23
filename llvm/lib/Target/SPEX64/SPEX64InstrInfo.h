//===-- SPEX64InstrInfo.h - SPEX64 instruction information --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H

#include "SPEX64RegisterInfo.h"

#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_INSTRINFO_HEADER
#include "SPEX64GenInstrInfo.inc"

namespace llvm {

class SPEX64InstrInfo : public SPEX64GenInstrInfo {
  SPEX64RegisterInfo RI;

public:
  SPEX64InstrInfo(const TargetSubtargetInfo &STI);
  const SPEX64RegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;
};

} // namespace llvm
#endif
