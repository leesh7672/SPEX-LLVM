//===-- SPEX64TargetMachineImpl.cpp - SPEX64 target machine implementation --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64ISelDAGToDAG.h"
#include "SPEX64TargetMachine.h"

#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

using namespace llvm;

static constexpr const char *SPEX64DataLayout =
    "e-m:e-p:64:64-i64:64-n8:16:32:64-S128";

SPEX64TargetMachine::SPEX64TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, SPEX64DataLayout, TT, CPU, FS, Options,
                               RM.value_or(Reloc::Static),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

SPEX64TargetMachine::~SPEX64TargetMachine() = default;

namespace {
class SPEX64PassConfig final : public TargetPassConfig {
public:
  SPEX64PassConfig(SPEX64TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  SPEX64TargetMachine &getSPEX64TM() const {
    return getTM<SPEX64TargetMachine>();
  }

  bool addInstSelector() override {
    addPass(createSPEX64ISelDag(getSPEX64TM()));
    return false;
  }

  FunctionPass *createTargetRegisterAllocator(bool) override {
    return createGreedyRegisterAllocator();
  }
};
} // namespace

TargetPassConfig *SPEX64TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SPEX64PassConfig(*this, PM);
}
