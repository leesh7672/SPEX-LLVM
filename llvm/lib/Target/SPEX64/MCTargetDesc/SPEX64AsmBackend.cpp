//===-- SPEX64AsmBackend.cpp - SPEX64 assembly backend --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64FixupKinds.h"
#include "SPEX64MCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace {
class SPEX64AsmBackend : public MCAsmBackend {
  Triple::OSType OSType;

public:
  bool shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup,
                             const MCValue &Target) {
    (void)Asm;
    // If the target is not fully resolved, let the object writer emit a
    // relocation.
    // Force relocations for symbolic branch/call immediates. Without this,
    // the assembler may treat cross-fragment symbols as unresolved but still
    // emit no relocation, leaving the immediate as 0 in the object file.
    //
    // We conservatively force relocations for data-sized fixups used by
    // SPEX64 variable-length encodings (imm32/imm64 payloads).
    switch (Fixup.getKind()) {
    case FK_Data_4:
    case FK_Data_8:
      return true;
    default:
      return false;
    }
  }

  explicit SPEX64AsmBackend(const MCSubtargetInfo &STI)
      : MCAsmBackend(endianness::little),
        OSType(STI.getTargetTriple().getOS()) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createSPEX64ELFObjectTargetWriter(OSABI);
  }

  bool shouldForceRelocation(const MCAssembler &, const MCFixup &Fixup,
                             const MCValue &) const {
    // Allow the assembler to resolve absolute fixups when the target is known
    // at assembly time (e.g. intra-section symbols). This avoids depending on
    // the linker to apply target-specific relocations for simple ROM images.
    switch (Fixup.getKind()) {
    case FK_Data_4:
    case FK_Data_8:
      return true;
    default:
      return false;
    }
  }

  bool needsRelocateWithSymbol(const MCValue &, const MCFixup &) const {
    return false;
  }

  void applyFixup(const MCFragment &Fragment, const MCFixup &Fixup,
                  const MCValue &Target, uint8_t *Data, uint64_t Value,
                  bool IsResolved) override {
    (void)IsResolved;

    (void)Fragment;

    const MCFixupKindInfo &Info = getFixupKindInfo(Fixup.getKind());
    if (Info.TargetSize == 0)
      return;

    (void)Target;
    (void)IsResolved;

    // The assembler computes the resolved value and passes it in as `Value`.
    // Do NOT use Target.getConstant() here: it is often 0 for symbolic fixups.
    Value >>= Info.TargetOffset;

    uint64_t Mask =
        Info.TargetSize == 64 ? ~0ULL : ((1ULL << Info.TargetSize) - 1);
    Value &= Mask;

    unsigned Offset = Fixup.getOffset();
    unsigned NumBytes = (Info.TargetSize + 7) / 8;
    for (unsigned I = 0; I < NumBytes; ++I) {
      Data[Offset + I] = static_cast<uint8_t>(Value >> (I * 8));
    }
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *) const override {
    const uint32_t W0 = 0;
    while (Count >= 4) {
      OS.write(reinterpret_cast<const char *>(&W0), 4);
      Count -= 4;
    }
    return Count == 0;
  }

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override {
    switch (Kind) {
    case (MCFixupKind)SPEX64::fixup_spex64_32:
      // 32-bit little-endian immediate.
      return {"fixup_spex64_32", 0, 32, 0};
    case (MCFixupKind)SPEX64::fixup_spex64_64:
      // 64-bit little-endian immediate.
      return {"fixup_spex64_64", 0, 64, 0};
    default:
      return MCAsmBackend::getFixupKindInfo(Kind);
    }
  }

  unsigned getRelocType(const MCFixup &Fixup) const {
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

MCAsmBackend *createSPEX64AsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &,
                                     const MCTargetOptions &) {
  return new SPEX64AsmBackend(STI);
}
} // namespace llvm