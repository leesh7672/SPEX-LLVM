//===-- SPEXELFObjectWriter.cpp - SPEX ELF object writer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXFixupKinds.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"

namespace llvm {
namespace {
class SPEXELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit SPEXELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit=*/true, OSABI, ELF::EM_SPEX,
                                /*HasRelocationAddend=*/true) {}
  ~SPEXELFObjectWriter() override = default;

  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override {
    switch (Type) {
    case ELF::R_SPEX_32:
    case ELF::R_SPEX_64:
      return true;
    default:
      return false;
    }
  }

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &,
                        bool) const override {
    switch (Fixup.getKind()) {
    case FK_Data_4:
      return ELF::R_SPEX_32;
    case FK_Data_8:
      return ELF::R_SPEX_64;
    case (MCFixupKind)SPEX::fixup_spex64_32:
      return ELF::R_SPEX_32;
    case (MCFixupKind)SPEX::fixup_spex64_64:
      return ELF::R_SPEX_64;
    default:
      return ELF::R_SPEX_NONE;
    }
  }
};
} // namespace

std::unique_ptr<MCObjectTargetWriter>
createSPEXELFObjectTargetWriter(uint8_t OSABI) {
  return std::make_unique<SPEXELFObjectWriter>(OSABI);
}

std::unique_ptr<MCObjectWriter> createSPEXELFObjectWriter(raw_pwrite_stream &OS,
                                                          uint8_t OSABI) {
  auto MOTW = std::make_unique<SPEXELFObjectWriter>(OSABI);
  return std::make_unique<ELFObjectWriter>(std::move(MOTW), OS,
                                           /*IsLittleEndian=*/true);
}
} // namespace llvm
