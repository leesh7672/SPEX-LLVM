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
  const MCInstrInfo &MCII;
  static constexpr uint64_t SPEXII_I64 = 1ull << 0;
  static constexpr uint64_t SPEXII_I1  = 1ull << 1;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getBranchTargetOpValue(const MCInst &MI, const MCOperand &MO,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;

public:
  SPEXMCCodeEmitter(MCContext &Ctx, const MCInstrInfo &MCII)
      : Ctx(Ctx), MCII(MCII) {}
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
      // For symbolic immediates, always emit a relocation so the linker can
      // resolve it (e.g. `call rmain` in a .o).
      //
      // Branch instructions already attach their own fixup via
      // getBranchTargetOpValue(). Avoid emitting a duplicate fixup here.
      const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
      const bool IsBranchImm =
          Desc.isBranch() && ((Desc.TSFlags & SPEXII_I1) != 0);
      if (!IsBranchImm) {
        // Use the generic data fixup kinds so we don't depend on custom fixup
        // handling for basic absolute addresses.
        MCFixupKind Kind = I64 ? FK_Data_8 : FK_Data_4;
        Fixups.push_back(MCFixup::create(/*Offset=*/4, ImmOp->getExpr(), Kind));
      }
    }
  }

  support::endian::write(CB, W0, endianness::little);
  if (I64) {
    support::endian::write(CB, Imm64, endianness::little);
  } else if (I1) {
    support::endian::write(CB, Imm32, endianness::little);
  }
}

unsigned
SPEXMCCodeEmitter::getBranchTargetOpValue(const MCInst &MI, const MCOperand &MO,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  (void)MI;
  (void)STI;

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  if (MO.isExpr()) {
    // Absolute address stored in the extension word(s) at byte offset +4.
    // Use 32- vs 64-bit fixup depending on the instruction encoding bit.
    const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
    const bool Is64 = (Desc.TSFlags & SPEXII_I64) != 0;
    MCFixupKind Kind =
        Is64 ? (MCFixupKind)SPEX::fixup_spex64_64
             : (MCFixupKind)SPEX::fixup_spex64_32;
    Fixups.push_back(MCFixup::create(4, MO.getExpr(), Kind));
    return 0;
  }

  llvm_unreachable("Unexpected operand kind in branch target");
}


} // namespace

MCCodeEmitter *createSPEXMCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx) {
  return new SPEXMCCodeEmitter(Ctx, MCII);
}

#include "SPEXGenMCCodeEmitter.inc"

} // namespace llvm
