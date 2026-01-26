//===-- SPEX64ELFObjectWriter.cpp - SPEX64 ELF object writer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"

namespace llvm {
namespace {
class SPEX64ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit SPEX64ELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit=*/true, OSABI, ELF::EM_SPEX64,
                                /*HasRelocationAddend=*/true) {}
  ~SPEX64ELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &, bool) const override {
    switch (Fixup.getKind()) {
    case FK_Data_4:
      return ELF::R_SPEX64_32;
    case FK_Data_8:
      return ELF::R_SPEX64_64;
    default:
      return ELF::R_SPEX64_NONE;
    }
  }
};
} // namespace

std::unique_ptr<MCObjectTargetWriter>
createSPEX64ELFObjectTargetWriter(uint8_t OSABI) {
  return std::make_unique<SPEX64ELFObjectWriter>(OSABI);
}

std::unique_ptr<MCObjectWriter>
createSPEX64ELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI) {
  auto MOTW = std::make_unique<SPEX64ELFObjectWriter>(OSABI);
  return std::make_unique<ELFObjectWriter>(std::move(MOTW), OS,
                                           /*IsLittleEndian=*/true);
}
} // namespace llvm
