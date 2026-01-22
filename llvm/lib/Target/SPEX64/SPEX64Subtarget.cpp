#include "SPEX64Subtarget.h"

using namespace llvm;

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SPEX64GenSubtargetInfo.inc"

SPEX64Subtarget::SPEX64Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                                     SPEX64TargetMachine &TM)
    : SPEX64GenSubtargetInfo(TT, CPU, CPU, FS), InstrInfo(*this),
      FrameLowering() {
  ParseSubtargetFeatures(CPU, CPU, FS);
}

SPEX64Subtarget::~SPEX64Subtarget() = default;

const TargetRegisterInfo *SPEX64Subtarget::getRegisterInfo() const {
  return &InstrInfo.getRegisterInfo();
}
