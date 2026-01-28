//===-- SPEXMCCodeEmitter.cpp - SPEX MC code emitter --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXFixupKinds.h"
#include "SPEXMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdint>

namespace llvm {
namespace {

class SPEXMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

public:
  SPEXMCCodeEmitter(MCContext &Ctx, const MCInstrInfo &) : Ctx(Ctx) {}
  ~SPEXMCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

unsigned
SPEXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  (void)MI;
  (void)Fixups;
  (void)STI;
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());
  if (MO.isExpr())
    return 0;
  return 0;
}

void SPEXMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  uint32_t W0 = static_cast<uint32_t>(getBinaryCodeForInstr(MI, Fixups, STI));

  bool I1 = (W0 >> 16) & 1;
  bool I64 = (W0 >> 15) & 1;

  if (!I1) {
    support::endian::write(CB, W0, endianness::little);
    return;
  }

  const MCOperand *ImmOp = nullptr;
  for (unsigned I = 0, E = MI.getNumOperands(); I < E; ++I) {
    const MCOperand &Op = MI.getOperand(I);
    if (Op.isExpr()) {
      ImmOp = &Op;
      break;
    }
  }
  if (!ImmOp) {
    for (unsigned I = 0, E = MI.getNumOperands(); I < E; ++I) {
      const MCOperand &Op = MI.getOperand(I);
      if (Op.isImm()) {
        ImmOp = &Op;
        break;
      }
    }
  }

  // Instructions with I1=1 normally carry an immediate/expr operand.
  // If this is missing, keep encoding with zero-fill but emit a diagnostic so
  // the root cause can be fixed in TableGen/ISel.
  if (!ImmOp) {
    errs() << "SPEX: I1 instruction missing Imm/Expr operand: opcode="
           << MI.getOpcode() << "\n";
    for (unsigned I = 0, E = MI.getNumOperands(); I < E; ++I) {
      const MCOperand &Op = MI.getOperand(I);
      errs() << "  op[" << I << "]: "
             << (Op.isReg()    ? "Reg"
                 : Op.isImm()  ? "Imm"
                 : Op.isExpr() ? "Expr"
                               : "Other")
             << "\n";
    }

    // Emit the base word and a zero-filled extension so the stream stays
    // well-formed, then stop.
    support::endian::write(CB, W0, endianness::little);
    if (I64)
      support::endian::write(CB, uint64_t(0), endianness::little);
    else
      support::endian::write(CB, uint32_t(0), endianness::little);
    return;
  }

  uint32_t Imm32 = 0;
  uint64_t Imm64 = 0;
  if (ImmOp) {
    if (ImmOp->isImm()) {
    Imm64 = static_cast<uint64_t>(ImmOp->getImm());
    Imm32 = static_cast<uint32_t>(Imm64);
    } else if (ImmOp->isExpr()) {
      MCFixupKind Kind = (!I64) ? (MCFixupKind)SPEX::fixup_spex64_32
                                : (MCFixupKind)SPEX::fixup_spex64_64;
      Fixups.push_back(MCFixup::create(/*Offset=*/4, ImmOp->getExpr(), Kind));
    }
  }

  support::endian::write(CB, W0, endianness::little);
  if (I64) {
    support::endian::write(CB, Imm64, endianness::little);
  } else if (I1) {
    support::endian::write(CB, Imm32, endianness::little);
  }
}

} // namespace

MCCodeEmitter *createSPEXMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  return new SPEXMCCodeEmitter(Ctx, MCII);
}

#include "SPEXGenMCCodeEmitter.inc"

} // namespace llvm
