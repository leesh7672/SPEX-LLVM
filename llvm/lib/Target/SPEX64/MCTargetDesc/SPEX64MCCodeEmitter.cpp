//===-- SPEX64MCCodeEmitter.cpp - SPEX64 MC code emitter --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64MCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/EndianStream.h"

#include <cstdint>

namespace llvm {
namespace {

class SPEX64MCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

public:
  SPEX64MCCodeEmitter(MCContext &Ctx, const MCInstrInfo &) : Ctx(Ctx) {}
  ~SPEX64MCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

unsigned
SPEX64MCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
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

void SPEX64MCCodeEmitter::encodeInstruction(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  uint32_t W0 = static_cast<uint32_t>(getBinaryCodeForInstr(MI, Fixups, STI));
  unsigned Size = 4;
  if (W0 & (1u << 16)) {
    Size = (W0 & (1u << 15)) ? 12 : 8;
  }
  const MCExpr *Expr = nullptr;
  for (unsigned I = 0, E = MI.getNumOperands(); I < E; ++I) {
    const MCOperand &Op = MI.getOperand(I);
    if (Op.isExpr()) {
      Expr = Op.getExpr();
      break;
    }
  }

  if (Size <= 4) {
    if (Expr)
      Fixups.push_back(MCFixup::create(0, Expr, FK_Data_4));
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

  uint32_t Imm32 = 0;
  uint64_t Imm64 = 0;
  if (ImmOp) {
    if (ImmOp->isImm()) {
      Imm64 = static_cast<uint64_t>(ImmOp->getImm());
      Imm32 = static_cast<uint32_t>(Imm64);
    } else if (ImmOp->isExpr()) {
      MCFixupKind Kind = (Size == 8) ? FK_Data_4 : FK_Data_8;
      Fixups.push_back(MCFixup::create(4, ImmOp->getExpr(), Kind));
    }
  }

  support::endian::write(CB, W0, endianness::little);
  if (Size == 8) {
    support::endian::write(CB, Imm32, endianness::little);
  } else if (Size == 12) {
    support::endian::write(CB, Imm64, endianness::little);
  }
}

} // namespace

MCCodeEmitter *createSPEX64MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  return new SPEX64MCCodeEmitter(Ctx, MCII);
}

#include "SPEX64GenMCCodeEmitter.inc"

} // namespace llvm
