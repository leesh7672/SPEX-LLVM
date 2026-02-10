//===- SPEXInstPrinter.cpp - SPEX MCInst Printer -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXInstPrinter.h"
#include "SPEXMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "SPEXGenAsmWriter.inc"

void SPEXInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << getRegisterName(Reg);
}

void SPEXInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O))
    printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void SPEXInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  if (OpNo >= MI->getNumOperands()) {
    O << "<illegal>";
    return;
  }
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    MAI.printExpr(O, *Op.getExpr());
  }
}
