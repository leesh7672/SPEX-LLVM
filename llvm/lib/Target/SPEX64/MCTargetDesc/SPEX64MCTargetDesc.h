#ifndef LLVM_LIB_TARGET_SPEX64_MCTARGETDESC_SPEX64MCTARGETDESC_H
#define LLVM_LIB_TARGET_SPEX64_MCTARGETDESC_SPEX64MCTARGETDESC_H

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCAsmInfo;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCInstPrinter;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class raw_pwrite_stream;

MCCodeEmitter *createSPEX64MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);
MCInstPrinter *createSPEX64MCInstPrinter(const Triple &T, unsigned SyntaxVariant,
                                         const MCAsmInfo &MAI,
                                         const MCInstrInfo &MII,
                                         const MCRegisterInfo &MRI);
MCAsmBackend *createSPEX64AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);
std::unique_ptr<MCObjectWriter>
createSPEX64ELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI);
std::unique_ptr<MCObjectTargetWriter>
createSPEX64ELFObjectTargetWriter(uint8_t OSABI);
} // namespace llvm

#define GET_REGINFO_ENUM
#include "../SPEX64GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "../SPEX64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "../SPEX64GenSubtargetInfo.inc"

#endif
