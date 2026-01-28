//===-- SPEXFixupKinds.h - SPEX Fixup Kinds --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_MCTARGETDESC_SPEXFIXUPKINDS_H
#define LLVM_LIB_TARGET_SPEX_MCTARGETDESC_SPEXFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace SPEX {

enum Fixups : unsigned {
  fixup_spex64_32 = FirstTargetFixupKind,
  fixup_spex64_64,
  fixup_spex64_num
};

} // namespace SPEX
} // namespace llvm

#endif
