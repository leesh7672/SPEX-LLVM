//===- SPEX64InstPrinter.h - SPEX64 MCInst Printer -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_MCTARGETDESC_SPEX64INSTPRINTER_H
#define LLVM_LIB_TARGET_SPEX64_MCTARGETDESC_SPEX64INSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class SPEX64InstPrinter : public MCInstPrinter {
public:
  SPEX64InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                    const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;
  void printRegName(raw_ostream &O, MCRegister Reg) override;

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
};

} // namespace llvm

#endif
