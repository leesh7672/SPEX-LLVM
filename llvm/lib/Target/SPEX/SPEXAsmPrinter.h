//===-- SPEXAsmPrinter.h - SPEX assembly printer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXASMPRINTER_H
#define LLVM_LIB_TARGET_SPEX_SPEXASMPRINTER_H

#include "SPEXMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class SPEXAsmPrinter final : public AsmPrinter {
  SPEXMCInstLower MCInstLowering;

public:
  SPEXAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer);
  ~SPEXAsmPrinter() override;

  StringRef getPassName() const override { return "SPEX Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
};

} // namespace llvm

#endif
