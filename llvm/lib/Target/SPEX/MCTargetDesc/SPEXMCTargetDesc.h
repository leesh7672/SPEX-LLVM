//===-- SPEXMCTargetDesc.h - SPEX MC target description --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_MCTARGETDESC_SPEXMCTARGETDESC_H
#define LLVM_LIB_TARGET_SPEX_MCTARGETDESC_SPEXMCTARGETDESC_H

#include "SPEXFixupKinds.h"
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

MCCodeEmitter *createSPEXMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);
MCInstPrinter *createSPEXMCInstPrinter(const Triple &T,
                                         unsigned SyntaxVariant,
                                         const MCAsmInfo &MAI,
                                         const MCInstrInfo &MII,
                                         const MCRegisterInfo &MRI);
MCAsmBackend *createSPEXAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);
std::unique_ptr<MCObjectWriter>
createSPEXELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI);
std::unique_ptr<MCObjectTargetWriter>
createSPEXELFObjectTargetWriter(uint8_t OSABI);
} // namespace llvm

#define GET_REGINFO_ENUM
#include "../SPEXGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "../SPEXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "../SPEXGenSubtargetInfo.inc"

#endif
