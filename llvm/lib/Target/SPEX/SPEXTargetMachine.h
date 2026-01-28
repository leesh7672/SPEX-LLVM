//===-- SPEXTargetMachine.h - SPEX target machine --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXTARGETMACHINE_H
#define LLVM_LIB_TARGET_SPEX_SPEXTARGETMACHINE_H

namespace llvm {
class SPEXTargetMachine;
} // namespace llvm

#include "SPEXSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class SPEXTargetMachine : public CodeGenTargetMachineImpl {
private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  SPEXSubtarget Subtarget;

public:
  SPEXTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);

  ~SPEXTargetMachine() override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  const SPEXSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif
