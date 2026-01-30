//===-- X86SelectionDAGInfo.h - X86 SelectionDAG Info -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the X86 subclass for SelectionDAGTargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_SPEX_SPEXSELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class SPEXSelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  explicit SPEXSelectionDAGInfo() = default;
};

} // namespace llvm

#endif
