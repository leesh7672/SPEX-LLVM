#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64SUBTARGET_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64SUBTARGET_H

#include "SPEX64FrameLowering.h"
#include "SPEX64InstrInfo.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/TargetParser/Triple.h"

#define GET_SUBTARGETINFO_HEADER
#include "SPEX64GenSubtargetInfo.inc"

namespace llvm {

class SPEX64Subtarget : public SPEX64GenSubtargetInfo {
public:
  enum Lane : unsigned { Lane0 = 0, Lane1 = 1, Lane2 = 2, Lane3 = 3 };

private:
  Lane CurLane = Lane0;

  SPEX64InstrInfo InstrInfo;
  SPEX64FrameLowering FrameLowering;

public:
  SPEX64Subtarget(const Triple &TT, StringRef CPU, StringRef FS);

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  void setLaneID(unsigned V) { CurLane = static_cast<Lane>(V & 3u); }
  Lane getLane() const { return CurLane; }

  const TargetInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetRegisterInfo *getRegisterInfo() const override;
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
};

} // namespace llvm
#endif
