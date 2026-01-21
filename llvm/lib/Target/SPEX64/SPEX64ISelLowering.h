#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ISELLOWERING_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SPEX64TargetMachine;
class SPEX64Subtarget;

class SPEX64TargetLowering : public TargetLowering {
public:
  explicit SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                const SPEX64Subtarget &ST);
};

} // namespace llvm

#endif
