//===-- SPEXTargetInfo.h - SPEX target info --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_TARGETINFO_SPEXTARGETINFO_H
#define LLVM_LIB_TARGET_SPEX_TARGETINFO_SPEXTARGETINFO_H

namespace llvm {
class Target;
Target &getTheSPEXTarget();
} // namespace llvm

#endif
