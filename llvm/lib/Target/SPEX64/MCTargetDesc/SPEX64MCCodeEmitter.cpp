#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/Endian.h"

#include <cstdint>
namespace llvm {
namespace {
class SPEX64MCCodeEmitter : public MCCodeEmitter {
public:
  explicit SPEX64MCCodeEmitter(const MCInstrInfo &) {}
  ~SPEX64MCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &Inst, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override {
    (void)Inst;
    (void)Fixups;
    (void)STI;
    uint32_t W0 = 0;
    char Buf[sizeof(W0)];
    support::endian::write32le(Buf, W0);
    CB.append(Buf, Buf + sizeof(Buf));
  }
};
} // namespace

MCCodeEmitter *createSPEX64MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  (void)Ctx;
  return new SPEX64MCCodeEmitter(MCII);
}
} // namespace llvm
