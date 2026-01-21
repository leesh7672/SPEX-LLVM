#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H

#include "SPEX64RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "SPEX64GenInstrInfo.inc"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

namespace llvm {

class SPEX64InstrInfo : public SPEX64GenInstrInfo {
  SPEX64RegisterInfo RI;

public:
  SPEX64InstrInfo();
  const SPEX64RegisterInfo &getRegisterInfo() const { return RI; }
};

} // namespace llvm
#endif
