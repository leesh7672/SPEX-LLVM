//===-- SPEX.h - SPEX target definitions --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEX_H
#define LLVM_LIB_TARGET_SPEX_SPEX_H

#include "llvm/Support/DataTypes.h"

#define GET_REGINFO_ENUM
#include "SPEXGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "SPEXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SPEXGenSubtargetInfo.inc"

namespace llvm {
class MachineFunctionPass;
class PassRegistry;
void initializeSPEXDAGToDAGISelLegacyPass(PassRegistry &);
MachineFunctionPass *createSPEXExpandPseudoPass();
MachineFunctionPass *createSPEXWrapCallsPass();
} // namespace llvm

#endif
