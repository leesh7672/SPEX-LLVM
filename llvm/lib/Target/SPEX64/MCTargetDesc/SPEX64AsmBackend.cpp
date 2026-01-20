#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class SPEX64AsmBackend : public MCAsmBackend {
public:
  SPEX64AsmBackend() : MCAsmBackend(endianness::little) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    llvm_unreachable("Use createSPEX64ELFObjectWriter()");
  }

  void applyFixup(const MCFragment &Fragment, const MCFixup &Fixup,
                  const MCValue &Target, uint8_t *Data, uint64_t,
                  bool) override {
    (void)Fragment;
    (void)Fixup;
    (void)Target;
    (void)Data;
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
    return MCAsmBackend::getFixupKindInfo(Kind);
  }
};
} // namespace

MCAsmBackend *createSPEX64AsmBackend(const Target &, const MCSubtargetInfo &,
                                     const MCRegisterInfo &,
                                     const MCTargetOptions &) {
  return new SPEX64AsmBackend();
}
