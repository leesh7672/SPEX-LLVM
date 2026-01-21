#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64FRAMELOWERING_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class SPEX64FrameLowering : public TargetFrameLowering {
public:
  SPEX64FrameLowering() : TargetFrameLowering(StackGrowsDown, Align(16), 0) {}

  void emitPrologue(MachineFunction &, MachineBasicBlock &) const override {}
  void emitEpilogue(MachineFunction &, MachineBasicBlock &) const override {}

private:
  bool hasFPImpl(const MachineFunction &) const override { return false; }
};
} // namespace llvm

#endif
