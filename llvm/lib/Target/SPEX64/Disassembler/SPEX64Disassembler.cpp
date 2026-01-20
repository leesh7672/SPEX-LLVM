//===- SPEX64Disassembler.cpp - Disassembler for SPEX64 ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../MCTargetDesc/SPEX64MCTargetDesc.h"
#include "../TargetInfo/SPEX64TargetInfo.h"
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

class SPEX64Disassembler : public MCDisassembler {
  mutable ArrayRef<uint8_t> CurBytes;

public:
  SPEX64Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~SPEX64Disassembler() override = default;

  void setCurBytes(ArrayRef<uint8_t> Bytes) const { CurBytes = Bytes; }
  ArrayRef<uint8_t> getCurBytes() const { return CurBytes; }

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

static const MCRegister GPRDecoderTable[] = {
    SPEX64::R0,  SPEX64::R1,  SPEX64::R2,  SPEX64::R3,  SPEX64::R4,  SPEX64::R5,
    SPEX64::R6,  SPEX64::R7,  SPEX64::R8,  SPEX64::R9,  SPEX64::R10, SPEX64::R11,
    SPEX64::R12, SPEX64::R13, SPEX64::R14, SPEX64::R15, SPEX64::R16, SPEX64::R17,
    SPEX64::R18, SPEX64::R19, SPEX64::R20, SPEX64::R21, SPEX64::R22, SPEX64::R23,
    SPEX64::R24, SPEX64::R25, SPEX64::R26, SPEX64::R27, SPEX64::R28, SPEX64::R29,
    SPEX64::R30, SPEX64::R31, SPEX64::R32, SPEX64::R33, SPEX64::R34, SPEX64::R35,
    SPEX64::R36, SPEX64::R37, SPEX64::R38, SPEX64::R39, SPEX64::R40, SPEX64::R41,
    SPEX64::R42, SPEX64::R43, SPEX64::R44, SPEX64::R45, SPEX64::R46, SPEX64::R47,
    SPEX64::R48, SPEX64::R49, SPEX64::R50, SPEX64::R51, SPEX64::R52, SPEX64::R53,
    SPEX64::R54, SPEX64::R55, SPEX64::R56, SPEX64::R57, SPEX64::R58, SPEX64::R59,
    SPEX64::R60, SPEX64::R61, SPEX64::R62, SPEX64::R63,
};

DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                    uint64_t /*Address*/,
                                    const MCDisassembler * /*Decoder*/) {
  if (RegNo >= std::size(GPRDecoderTable))
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

DecodeStatus DecodeSPEX64Imm9(MCInst &Inst, unsigned Imm, uint64_t /*Address*/,
                              const MCDisassembler * /*Decoder*/) {
  Inst.addOperand(MCOperand::createImm(Imm & 0x1FFu));
  return MCDisassembler::Success;
}

static DecodeStatus decodeImmediate32(MCInst &Inst,
                                      const MCDisassembler *Decoder) {
  auto *Dis = static_cast<const SPEX64Disassembler *>(Decoder);
  ArrayRef<uint8_t> Bytes = Dis->getCurBytes();
  if (Bytes.size() < 8)
    return MCDisassembler::Fail;
  uint32_t Imm = support::endian::read32le(Bytes.data() + 4);
  Inst.addOperand(MCOperand::createImm(static_cast<uint64_t>(Imm)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeImmediate64(MCInst &Inst,
                                      const MCDisassembler *Decoder) {
  auto *Dis = static_cast<const SPEX64Disassembler *>(Decoder);
  ArrayRef<uint8_t> Bytes = Dis->getCurBytes();
  if (Bytes.size() < 12)
    return MCDisassembler::Fail;
  uint64_t Imm = support::endian::read64le(Bytes.data() + 4);
  Inst.addOperand(MCOperand::createImm(static_cast<int64_t>(Imm)));
  return MCDisassembler::Success;
}

DecodeStatus DecodeSPEX64Imm32(MCInst &Inst, unsigned /*Imm*/,
                               uint64_t /*Address*/,
                               const MCDisassembler *Decoder) {
  return decodeImmediate32(Inst, Decoder);
}

DecodeStatus DecodeSPEX64Imm64(MCInst &Inst, unsigned /*Imm*/,
                               uint64_t /*Address*/,
                               const MCDisassembler *Decoder) {
  return decodeImmediate64(Inst, Decoder);
}

#include "SPEX64GenDisassemblerTables.inc"

DecodeStatus SPEX64Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
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

static MCDisassembler *createSPEX64Disassembler(const Target & /*T*/,
                                                const MCSubtargetInfo &STI,
                                                MCContext &Ctx) {
  return new SPEX64Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPEX64Disassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheSPEX64Target(),
                                         createSPEX64Disassembler);
}
