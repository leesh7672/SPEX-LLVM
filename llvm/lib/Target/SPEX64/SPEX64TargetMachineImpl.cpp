#include "SPEX64ISelDAGToDAG.h"
#include "SPEX64TargetMachine.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"

using namespace llvm;

SPEX64TargetMachine::SPEX64TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : TargetMachine(T, "e-m:e-p:64:64-i64:64-n8:16:32:64-S128", TT, CPU, FS,
                    Options) {}

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
};
} // namespace

TargetPassConfig *SPEX64TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SPEX64PassConfig(*this, PM);
}