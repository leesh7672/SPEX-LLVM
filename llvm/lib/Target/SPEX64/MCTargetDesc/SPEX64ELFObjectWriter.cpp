#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

namespace {
class SPEX64ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit SPEX64ELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit=*/true, OSABI, ELF::EM_NONE,
                                /*HasRelocationAddend=*/true) {}
  ~SPEX64ELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool) const override {
    return ELF::R_NONE;
  }
};
} // namespace

std::unique_ptr<MCObjectWriter>
createSPEX64ELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI) {
  auto MOTW = std::make_unique<SPEX64ELFObjectWriter>(OSABI);
  return std::make_unique<ELFObjectWriter>(std::move(MOTW), OS,
                                           /*IsLittleEndian=*/true);
}
