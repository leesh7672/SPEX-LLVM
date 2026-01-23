//===-- SPEX64MCInstLower.h - SPEX64 MC instruction lowering --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64MCINSTLOWER_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64MCINSTLOWER_H

namespace llvm {

class AsmPrinter;
class MachineInstr;
class MachineOperand;
class MCOperand;
class MCContext;
class MCInst;
class MCSymbol;

class SPEX64MCInstLower {
  MCContext &Ctx;
  AsmPrinter &Printer;

  MCSymbol *getGlobalAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *getExternalSymbolSymbol(const MachineOperand &MO) const;
  MCSymbol *getBlockAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *getJumpTableSymbol(const MachineOperand &MO) const;
  MCSymbol *getConstantPoolSymbol(const MachineOperand &MO) const;
  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

public:
  SPEX64MCInstLower(MCContext &Ctx, AsmPrinter &Printer);

  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
};

} // namespace llvm

#endif
