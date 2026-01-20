#include "../TargetInfo/SPEX64TargetInfo.h"
#include "SPEX64MCTargetDesc.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "SPEX64GenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SPEX64GenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SPEX64GenSubtargetInfo.inc"

MCInstrInfo *createSPEX64MCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitSPEX64MCInstrInfo(X);
  return X;
}

MCRegisterInfo *createSPEX64MCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitSPEX64MCRegisterInfo(X, /*RAReg=*/0);
  return X;
}

MCSubtargetInfo *createSPEX64MCSubtargetInfo(const Triple &TT, StringRef CPU,
                                             StringRef FS) {
  return createSPEX64MCSubtargetInfoImpl(TT, CPU, FS);
}

MCAsmInfo *createSPEX64MCAsmInfo(const MCRegisterInfo &, const Triple &,
                                 const MCTargetOptions &);
MCCodeEmitter *createSPEX64MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);
MCAsmBackend *createSPEX64AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);
std::unique_ptr<MCObjectWriter> createSPEX64ELFObjectWriter(raw_pwrite_stream &OS,
                                                           uint8_t OSABI);

extern "C" void LLVMInitializeSPEX64TargetMC() {
  const Target &T = getTheSPEX64Target();

  RegisterMCAsmInfoFn X(T, createSPEX64MCAsmInfo);

  TargetRegistry::RegisterMCInstrInfo(T, createSPEX64MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createSPEX64MCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createSPEX64MCSubtargetInfo);

  TargetRegistry::RegisterMCCodeEmitter(T, createSPEX64MCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(T, createSPEX64AsmBackend);

  TargetRegistry::RegisterMCObjectWriter(
      T, [](const Triple &, uint8_t OSABI, raw_pwrite_stream &OS) {
        return createSPEX64ELFObjectWriter(OS, OSABI);
      });
}
