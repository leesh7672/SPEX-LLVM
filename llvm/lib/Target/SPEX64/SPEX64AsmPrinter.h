//===-- SPEX64AsmPrinter.h - SPEX64 assembly printer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ASMPRINTER_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ASMPRINTER_H

#include "SPEX64MCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class SPEX64AsmPrinter final : public AsmPrinter {
  SPEX64MCInstLower MCInstLowering;

public:
  SPEX64AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer);
  ~SPEX64AsmPrinter() override;

  StringRef getPassName() const override { return "SPEX64 Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
};

} // namespace llvm

#endif
