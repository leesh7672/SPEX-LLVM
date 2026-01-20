#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64TARGETMACHINE_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64TARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {

class SPEX64TargetMachine : public TargetMachine {
public:
  SPEX64TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM,
                      CodeGenOptLevel OL, bool JIT);

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
};

} // namespace llvm

#endif
