//===-- SPEX64.h - SPEX64 target definitions --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64_H

#include "llvm/Support/DataTypes.h"

#define GET_REGINFO_ENUM
#include "SPEX64GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "SPEX64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SPEX64GenSubtargetInfo.inc"

namespace llvm {
class PassRegistry;
void initializeSPEX64DAGToDAGISelLegacyPass(PassRegistry &);
} // namespace llvm

#endif
