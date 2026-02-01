//===-- SPEXMCInstLower.cpp - Lower SPEX MachineInstr to MCInst
//--------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXMCInstLower.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

SPEXMCInstLower::SPEXMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
    : Ctx(Ctx), Printer(Printer) {}

MCSymbol *
SPEXMCInstLower::getGlobalAddressSymbol(const MachineOperand &MO) const {
  if (MO.getTargetFlags() != 0)
    report_fatal_error("SPEX: unsupported target flags on global");
  return Printer.getSymbol(MO.getGlobal());
}

MCSymbol *
SPEXMCInstLower::getExternalSymbolSymbol(const MachineOperand &MO) const {
  if (MO.getTargetFlags() != 0)
    report_fatal_error("SPEX: unsupported target flags on extern");
  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCSymbol *
SPEXMCInstLower::getBlockAddressSymbol(const MachineOperand &MO) const {
  if (MO.getTargetFlags() != 0)
    report_fatal_error("SPEX: unsupported target flags on blockaddress");
  return Printer.GetBlockAddressSymbol(MO.getBlockAddress());
}

MCSymbol *SPEXMCInstLower::getJumpTableSymbol(const MachineOperand &MO) const {
  if (MO.getTargetFlags() != 0)
    report_fatal_error("SPEX: unsupported target flags on jumptable");
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<64> Name;
  raw_svector_ostream(Name)
      << DL.getPrivateGlobalPrefix() << "JTI" << Printer.getFunctionNumber()
      << '_' << MO.getIndex();
  return Ctx.getOrCreateSymbol(Name);
}

MCSymbol *
SPEXMCInstLower::getConstantPoolSymbol(const MachineOperand &MO) const {
  if (MO.getTargetFlags() != 0)
    report_fatal_error("SPEX: unsupported target flags on constpool");
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<64> Name;
  raw_svector_ostream(Name)
      << DL.getPrivateGlobalPrefix() << "CPI" << Printer.getFunctionNumber()
      << '_' << MO.getIndex();
  return Ctx.getOrCreateSymbol(Name);
}

MCOperand SPEXMCInstLower::lowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol *Sym) const {
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);
  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  return MCOperand::createExpr(Expr);
}

void SPEXMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {

  if (MI->isDebugInstr())
    return;

  OutMI.setOpcode(MI->getOpcode());

  const MCInstrDesc &Desc = MI->getDesc();
  const unsigned ExpectedOps = Desc.getNumOperands();
  unsigned OpIdx = 0;

  // Do not silently lower pseudos/target-independent opcodes to garbage.
  if (MI->isPseudo()) {
    MI->print(errs());
    report_fatal_error("SPEX: pseudo reached MCInstLower; missing expansion");
  }
  if (MI->getOpcode() == TargetOpcode::COPY ||
      MI->getOpcode() == TargetOpcode::INSERT_SUBREG ||
      MI->getOpcode() == TargetOpcode::EXTRACT_SUBREG ||
      MI->getOpcode() == TargetOpcode::SUBREG_TO_REG ||
      MI->getOpcode() == TargetOpcode::IMPLICIT_DEF) {
    MI->print(errs());
    report_fatal_error("SPEX: unexpected TargetOpcode reached MCInstLower");
  }

  for (const MachineOperand &MO : MI->operands()) {
    if (MO.isMetadata())
      continue;
    if (MO.isReg() && MO.isImplicit())
      continue;
    if (MO.isRegMask())
      continue;
    if (OpIdx >= ExpectedOps)
      continue;

    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      MI->print(errs());
      report_fatal_error("SPEX: unknown operand type");
    case MachineOperand::MO_Register:
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = lowerSymbolOperand(MO, getGlobalAddressSymbol(MO));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = lowerSymbolOperand(MO, getExternalSymbolSymbol(MO));
      break;
    case MachineOperand::MO_BlockAddress:
      MCOp = lowerSymbolOperand(MO, getBlockAddressSymbol(MO));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = lowerSymbolOperand(MO, getJumpTableSymbol(MO));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = lowerSymbolOperand(MO, getConstantPoolSymbol(MO));
      break;
    }
    OutMI.addOperand(MCOp);
    ++OpIdx;
  }
}
