#include "SPEX64ISelLowering.h"
#include "SPEX64.h"
#include "SPEX64Subtarget.h"
#include "SPEX64TargetMachine.h"

using namespace llvm;

SPEX64TargetLowering::SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                           const SPEX64Subtarget &ST)
    : TargetLowering(TM, ST), ST(ST) {}