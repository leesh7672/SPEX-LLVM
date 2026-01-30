//===- SPEXDisassembler.cpp - Disassembler for SPEX ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "spex-disassembler"
#include "../MCTargetDesc/SPEXMCTargetDesc.h"
#include "../TargetInfo/SPEXTargetInfo.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/LEB128.h"

#include <iterator>
using namespace llvm;
using namespace llvm::MCD;

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class SPEXDisassembler : public MCDisassembler {
  mutable ArrayRef<uint8_t> CurBytes;

public:
  SPEXDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~SPEXDisassembler() override = default;

  void setCurBytes(ArrayRef<uint8_t> Bytes) const { CurBytes = Bytes; }
  ArrayRef<uint8_t> getCurBytes() const { return CurBytes; }

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

static const MCRegister GPRDecoderTable[] = {
    SPEX::R0,  SPEX::R1,  SPEX::R2,  SPEX::R3,  SPEX::R4,  SPEX::R5,  SPEX::R6,
    SPEX::R7,  SPEX::R8,  SPEX::R9,  SPEX::R10, SPEX::R11, SPEX::R12, SPEX::R13,
    SPEX::R14, SPEX::R15, SPEX::R16, SPEX::R17, SPEX::R18, SPEX::R19, SPEX::R20,
    SPEX::R21, SPEX::R22, SPEX::R23, SPEX::R24, SPEX::R25, SPEX::R26, SPEX::R27,
    SPEX::R28, SPEX::R29, SPEX::R30, SPEX::R31, SPEX::R32, SPEX::R33, SPEX::R34,
    SPEX::R35, SPEX::R36, SPEX::R37, SPEX::R38, SPEX::R39, SPEX::R40, SPEX::R41,
    SPEX::R42, SPEX::R43, SPEX::R44, SPEX::R45, SPEX::R46, SPEX::R47, SPEX::R48,
    SPEX::R49, SPEX::R50, SPEX::R51, SPEX::R52, SPEX::R53, SPEX::R54, SPEX::R55,
    SPEX::R56, SPEX::R57, SPEX::R58, SPEX::R59, SPEX::R60, SPEX::R61, SPEX::R62,
    SPEX::R63,
};

DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                    uint64_t /*Address*/,
                                    const MCDisassembler * /*Decoder*/) {
  if (RegNo >= std::size(GPRDecoderTable))
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

DecodeStatus DecodeSPEXImm9(MCInst &Inst, unsigned Imm, uint64_t /*Address*/,
                            const MCDisassembler * /*Decoder*/) {
  Inst.addOperand(MCOperand::createImm(Imm & 0x1FFu));
  return MCDisassembler::Success;
}

static DecodeStatus decodeImmediate32(MCInst &Inst,
                                      const MCDisassembler *Decoder) {
  auto *Dis = static_cast<const SPEXDisassembler *>(Decoder);
  ArrayRef<uint8_t> Bytes = Dis->getCurBytes();
  if (Bytes.size() < 8)
    return MCDisassembler::Fail;
  uint32_t Imm = support::endian::read32le(Bytes.data() + 4);
  Inst.addOperand(MCOperand::createImm(static_cast<uint64_t>(Imm)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeImmediate64(MCInst &Inst,
                                      const MCDisassembler *Decoder) {
  auto *Dis = static_cast<const SPEXDisassembler *>(Decoder);
  ArrayRef<uint8_t> Bytes = Dis->getCurBytes();
  if (Bytes.size() < 12)
    return MCDisassembler::Fail;
  uint64_t Imm = support::endian::read64le(Bytes.data() + 4);
  Inst.addOperand(MCOperand::createImm(static_cast<int64_t>(Imm)));
  return MCDisassembler::Success;
}

DecodeStatus DecodeSPEXImm32(MCInst &Inst, unsigned /*Imm*/,
                             uint64_t /*Address*/,
                             const MCDisassembler *Decoder) {
  return decodeImmediate32(Inst, Decoder);
}

DecodeStatus DecodeSPEXImm64(MCInst &Inst, unsigned /*Imm*/,
                             uint64_t /*Address*/,
                             const MCDisassembler *Decoder) {
  return decodeImmediate64(Inst, Decoder);
}

static DecodeStatus decodeImmediateVar(MCInst &Inst, uint64_t /*Address*/,
                                       const MCDisassembler *Decoder) {
  auto *Dis = static_cast<const SPEXDisassembler *>(Decoder);
  ArrayRef<uint8_t> Bytes = Dis->getCurBytes();
  if (Bytes.size() < 4)
    return MCDisassembler::Fail;

  uint32_t Insn = support::endian::read32le(Bytes.data());
  unsigned I1 = (Insn >> 16) & 0x1;
  unsigned I64 = (Insn >> 15) & 0x1;

  if (!I1)
    return MCDisassembler::Fail;

  if (I64) {
    if (Bytes.size() < 12)
      return MCDisassembler::Fail;
    uint64_t Imm = support::endian::read64le(Bytes.data() + 4);
    Inst.addOperand(MCOperand::createImm(static_cast<int64_t>(Imm)));
    return MCDisassembler::Success;
  }

  if (Bytes.size() < 8)
    return MCDisassembler::Fail;
  uint32_t Imm = support::endian::read32le(Bytes.data() + 4);
  Inst.addOperand(
      MCOperand::createImm(static_cast<int64_t>(static_cast<uint64_t>(Imm))));
  return MCDisassembler::Success;
}

DecodeStatus DecodeSPEXImmVar(MCInst &Inst, unsigned /*Imm*/, uint64_t Address,
                              const MCDisassembler *Decoder) {
  return decodeImmediateVar(Inst, Address, Decoder);
}

#include "SPEXGenDisassemblerTables.inc"

DecodeStatus SPEXDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  uint32_t Insn = support::endian::read32le(Bytes.data());
  setCurBytes(Bytes);

  DecodeStatus Result =
      decodeInstruction(DecoderTable32, Instr, Insn, Address, this, STI);
  if (Result != MCDisassembler::Success) {
    Size = 4;
    return Result;
  }

  Size = 4;
  unsigned I1 = (Insn >> 16) & 0x1;
  if (I1) {
    unsigned I64 = (Insn >> 15) & 0x1;
    Size = I64 ? 12 : 8;
    if (Bytes.size() < Size)
      return MCDisassembler::Fail;
  }

  return MCDisassembler::Success;
}

} // end anonymous namespace

static MCDisassembler *createSPEXDisassembler(const Target & /*T*/,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new SPEXDisassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPEXDisassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheSPEXTarget(),
                                         createSPEXDisassembler);
}
