//===-- SPEXMCTargetDesc.cpp - SPEX MC target description --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXMCTargetDesc.h"
#include "../TargetInfo/SPEXTargetInfo.h"
#include "SPEXInstPrinter.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

namespace llvm {
MCAsmInfo *createSPEXMCAsmInfo(const MCRegisterInfo &, const Triple &,
                                 const MCTargetOptions &);
} // namespace llvm

#define GET_INSTRINFO_MC_DESC
#include "SPEXGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SPEXGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SPEXGenSubtargetInfo.inc"

MCInstrInfo *createSPEXMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitSPEXMCInstrInfo(X);
  return X;
}

MCRegisterInfo *createSPEXMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitSPEXMCRegisterInfo(X, /*RAReg=*/0);
  return X;
}

MCSubtargetInfo *createSPEXMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                             StringRef FS) {
  return createSPEXMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

namespace llvm {
MCInstPrinter *createSPEXMCInstPrinter(const Triple &T,
                                         unsigned SyntaxVariant,
                                         const MCAsmInfo &MAI,
                                         const MCInstrInfo &MII,
                                         const MCRegisterInfo &MRI) {
  return new SPEXInstPrinter(MAI, MII, MRI);
}
} // namespace llvm

extern "C" void LLVMInitializeSPEXTargetMC() {
  Target &T = getTheSPEXTarget();

  RegisterMCAsmInfoFn X(T, createSPEXMCAsmInfo);

  TargetRegistry::RegisterMCInstrInfo(T, createSPEXMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createSPEXMCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createSPEXMCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createSPEXMCInstPrinter);

  TargetRegistry::RegisterMCCodeEmitter(T, createSPEXMCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createSPEXAsmBackend);
}
