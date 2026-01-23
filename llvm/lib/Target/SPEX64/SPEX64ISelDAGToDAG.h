//===-- SPEX64ISelDAGToDAG.h - SPEX64 DAG-to-DAG selector --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ISELDAGTODAG_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ISELDAGTODAG_H

namespace llvm {
class SPEX64TargetMachine;
class FunctionPass;
FunctionPass *createSPEX64ISelDag(SPEX64TargetMachine &TM);
} // namespace llvm

#endif
