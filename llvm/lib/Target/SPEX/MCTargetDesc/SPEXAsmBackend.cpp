//===-- SPEXInstInfo.td - SPEX instruction properties --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXFixupKinds.h"
#include "SPEXMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace {
class SPEXAsmBackend : public MCAsmBackend {
  Triple::OSType OSType;

public:
  bool shouldForceRelocation(const MCFixup &Fixup, const MCValue &Target) {
    (void)Target;
    // Keep relocations for these fixups in .o so that the linker can
    // resolve absolute addresses (calls, branches, and address
    // materialization).
    switch (Fixup.getKind()) {
    case (MCFixupKind)SPEX::fixup_spex64_32:
    case (MCFixupKind)SPEX::fixup_spex64_64:
    case FK_Data_4:
    case FK_Data_8:
      return true;
    default:
      return false;
    }
  }

  explicit SPEXAsmBackend(const MCSubtargetInfo &STI)
      : MCAsmBackend(endianness::little),
        OSType(STI.getTargetTriple().getOS()) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createSPEXELFObjectTargetWriter(OSABI);
  }

  bool shouldForceRelocation(const MCAssembler &, const MCFixup &Fixup,
                             const MCValue &) const {
    // Allow the assembler to resolve absolute fixups when the target is known
    // at assembly time (e.g. intra-section symbols). This avoids depending on
    // the linker to apply target-specific relocations for simple ROM images.
    switch (Fixup.getKind()) {
    case FK_Data_4:
    case FK_Data_8:
    case (MCFixupKind)SPEX::fixup_spex64_32:
    case (MCFixupKind)SPEX::fixup_spex64_64:
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
    case (MCFixupKind)SPEX::fixup_spex64_32:
      // 32-bit little-endian immediate.
      return {"fixup_spex64_32", 0, 32, 0};
    case (MCFixupKind)SPEX::fixup_spex64_64:
      // 64-bit little-endian immediate.
      return {"fixup_spex64_64", 0, 64, 0};
    default:
      return MCAsmBackend::getFixupKindInfo(Kind);
    }
  }

  unsigned getRelocType(const MCFixup &Fixup) const {
    switch (Fixup.getKind()) {
    case SPEX::fixup_spex64_32:
    case FK_Data_4:
      return ELF::R_SPEX_32;
    case SPEX::fixup_spex64_64:
    case FK_Data_8:
      return ELF::R_SPEX_64;
    default:
      return ELF::R_SPEX_NONE;
    }
  }
};
} // namespace

MCAsmBackend *createSPEXAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &,
                                     const MCTargetOptions &) {
  return new SPEXAsmBackend(STI);
}
} // namespace llvm