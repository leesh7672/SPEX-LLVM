#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64INSTRINFO_H

#include "SPEX64RegisterInfo.h"

#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_INSTRINFO_HEADER
#include "SPEX64GenInstrInfo.inc"

namespace llvm {

class SPEX64InstrInfo : public SPEX64GenInstrInfo {
  SPEX64RegisterInfo RI;

public:
  SPEX64InstrInfo(const TargetSubtargetInfo &STI);
  const SPEX64RegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;
};

} // namespace llvm
#endif
