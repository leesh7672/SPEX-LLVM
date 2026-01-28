//===-- SPEXAsmPrinter.cpp - SPEX assembly printer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXAsmPrinter.h"
#include "TargetInfo/SPEXTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

SPEXAsmPrinter::SPEXAsmPrinter(TargetMachine &TM,
                                   std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)), MCInstLowering(OutContext, *this) {}

void SPEXAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

SPEXAsmPrinter::~SPEXAsmPrinter() = default;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPEXAsmPrinter() {
  RegisterAsmPrinter<SPEXAsmPrinter> X(getTheSPEXTarget());
}
