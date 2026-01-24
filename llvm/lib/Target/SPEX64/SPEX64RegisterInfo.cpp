//===-- SPEX64RegisterInfo.cpp - SPEX64 Register Information ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64RegisterInfo.h"
#include "SPEX64.h"
#include "SPEX64FrameLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "SPEX64GenRegisterInfo.inc"

using namespace llvm;

SPEX64RegisterInfo::SPEX64RegisterInfo()
    : SPEX64GenRegisterInfo(/*RA=*/SPEX64::LR) {}

// Stack pointer selection.
// For v1 hardware, registers are globally accessible across lanes.
Register SPEX64RegisterInfo::getStackRegister(const MachineFunction &) const {
  return SPEX64::R63; // ABI stack pointer (convention)
}

// Frame pointer equals stack pointer.
Register SPEX64RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getStackRegister(MF);
}

// Reserved registers.
// NOTE:
//  - RX is the accumulator and must remain allocatable.
//  - No lane-based register reservation is applied.
//  - Only SP is reserved here.
BitVector SPEX64RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // Reserve stack pointer
  Reserved.set(getStackRegister(MF));

  // Reserve link register (return address)
  Reserved.set(SPEX64::LR);

  return Reserved;
}

const MCPhysReg *
SPEX64RegisterInfo::getCalleeSavedRegs(const MachineFunction *) const {
  return CSR_SPEX64_SaveList;
}

const uint32_t *
SPEX64RegisterInfo::getCallPreservedMask(const MachineFunction &,
                                         CallingConv::ID) const {
  return CSR_SPEX64_RegMask;
}

bool SPEX64RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                             int, unsigned FIOperandNum,
                                             RegScavenger *) const {

  MachineFunction &MF = *MI->getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  int FrameIndex = MI->getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(FrameIndex) +
                   MI->getOperand(FIOperandNum + 1).getImm() +
                   MFI.getStackSize();

  MI->getOperand(FIOperandNum)
      .ChangeToRegister(getFrameRegister(MF), /*isDef=*/false);
  MI->getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}
