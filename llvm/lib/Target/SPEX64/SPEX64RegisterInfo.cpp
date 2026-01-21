#include "SPEX64RegisterInfo.h"
#include "SPEX64.h"
#include "SPEX64Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

SPEX64RegisterInfo::SPEX64RegisterInfo() : SPEX64GenRegisterInfo(/*RA*/ 0) {}

static bool isInLaneWindow(unsigned Reg, SPEX64Subtarget::Lane L) {
  const unsigned idx = Reg - SPEX64::R0;
  if (idx > 63)
    return false;
  const unsigned base = static_cast<unsigned>(L) * 16;
  return idx >= base && idx < base + 16;
}

Register SPEX64RegisterInfo::getStackRegister(const MachineFunction &MF) const {
  const auto &ST = MF.getSubtarget<SPEX64Subtarget>();
  switch (ST.getLane()) {
  case SPEX64Subtarget::Lane0:
    return SPEX64::R13;
  case SPEX64Subtarget::Lane1:
    return SPEX64::R29;
  case SPEX64Subtarget::Lane2:
    return SPEX64::R45;
  case SPEX64Subtarget::Lane3:
    return SPEX64::R61;
  }
  llvm_unreachable("bad lane");
}

BitVector SPEX64RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const auto &ST = MF.getSubtarget<SPEX64Subtarget>();

  for (unsigned R = SPEX64::R0; R <= SPEX64::R63; ++R)
    if (!isInLaneWindow(R, ST.getLane()))
      Reserved.set(R);

  Reserved.set(getStackRegister(MF));

  switch (ST.getLane()) {
  case SPEX64Subtarget::Lane0:
    Reserved.set(SPEX64::R14);
    Reserved.set(SPEX64::R15);
    break;
  case SPEX64Subtarget::Lane1:
    Reserved.set(SPEX64::R30);
    Reserved.set(SPEX64::R31);
    break;
  case SPEX64Subtarget::Lane2:
    Reserved.set(SPEX64::R46);
    Reserved.set(SPEX64::R47);
    break;
  case SPEX64Subtarget::Lane3:
    Reserved.set(SPEX64::R62);
    Reserved.set(SPEX64::R63);
    break;
  }

  return Reserved;
}

Register SPEX64RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getStackRegister(MF);
}

const MCPhysReg *
SPEX64RegisterInfo::getCalleeSavedRegs(const MachineFunction *) const {
  return nullptr;
}

bool SPEX64RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                             int, unsigned FIOperandNum,
                                             RegScavenger *) const {
  report_fatal_error("SPEX64 eliminateFrameIndex not implemented yet");
}
