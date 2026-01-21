#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64_H

#include "llvm/Support/DataTypes.h"

#define GET_REGINFO_ENUM
#include "SPEX64GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "SPEX64GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SPEX64GenSubtargetInfo.inc"

#endif
