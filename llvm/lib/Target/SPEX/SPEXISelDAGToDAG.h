//===-- SPEXISelDAGToDAG.h - SPEX DAG-to-DAG selector --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXISELDAGTODAG_H
#define LLVM_LIB_TARGET_SPEX_SPEXISELDAGTODAG_H

namespace llvm {
class SPEXTargetMachine;
class FunctionPass;
FunctionPass *createSPEXISelDag(SPEXTargetMachine &TM);
} // namespace llvm

#endif
