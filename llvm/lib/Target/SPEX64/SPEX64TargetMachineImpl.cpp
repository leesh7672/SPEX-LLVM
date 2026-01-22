#include "SPEX64ISelDAGToDAG.h"
#include "SPEX64TargetMachine.h"

#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

using namespace llvm;

static constexpr const char *SPEX64DataLayout = "e-m:e-i64:64-n64";

SPEX64TargetMachine::SPEX64TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, SPEX64DataLayout, TT, CPU, FS, Options,
                               RM.value(), CM.value(), OL),
      TLOF(std::unique_ptr<TargetLoweringObjectFileELF>()), Subtarget(TT, CPU, FS, *this) {}

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
};
} // namespace

TargetPassConfig *SPEX64TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SPEX64PassConfig(*this, PM);
}
