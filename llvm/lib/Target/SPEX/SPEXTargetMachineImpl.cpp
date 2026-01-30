//===-- SPEXTargetMachineImpl.cpp - SPEX target machine implementation --*-
// C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX.h"
#include "SPEXISelDAGToDAG.h"
#include "SPEXTargetMachine.h"
#include "SPEXTargetObjectFile.h"

#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

static constexpr char SPEXDataLayout[] =
  "e-m:e-p:64:64-i64:64-i128:128-n64-S128";

SPEXTargetMachine::SPEXTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, SPEXDataLayout, TT, CPU, FS, Options,
                               RM.value_or(Reloc::Static),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<SPEXTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

SPEXTargetMachine::~SPEXTargetMachine() = default;

namespace {
class SPEXPassConfig final : public TargetPassConfig {
public:
  SPEXPassConfig(SPEXTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  SPEXTargetMachine &getSPEXTM() const { return getTM<SPEXTargetMachine>(); }

  bool addInstSelector() override {
    addPass(createSPEXISelDag(getSPEXTM()));
    return false;
  }

  FunctionPass *createTargetRegisterAllocator(bool Optimized) override {
    if (!Optimized)
      return createFastRegisterAllocator();
    return createGreedyRegisterAllocator();
  }
  void addPreEmitPass() override {
    addPass(createSPEXWrapCallsPass());
    addPass(createSPEXExpandPseudoPass());
  }
};
} // namespace

TargetPassConfig *SPEXTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SPEXPassConfig(*this, PM);
}
