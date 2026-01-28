//===-- SPEXRegisterInfo.cpp - SPEX Register Information ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXRegisterInfo.h"
#include "SPEX.h"
#include "SPEXFrameLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "SPEXGenRegisterInfo.inc"

using namespace llvm;

SPEXRegisterInfo::SPEXRegisterInfo()
    : SPEXGenRegisterInfo(/*RA=*/SPEX::LR) {}

// Stack pointer selection.
// For v1 hardware, registers are globally accessible across lanes.
Register SPEXRegisterInfo::getStackRegister(const MachineFunction &) const {
  return SPEX::R63; // ABI stack pointer (convention)
}

// Frame pointer equals stack pointer.
Register SPEXRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getStackRegister(MF);
}

// Reserved registers.
// NOTE:
//  - RX is the accumulator and must remain allocatable.
//  - No lane-based register reservation is applied.
//  - Only SP is reserved here.
BitVector SPEXRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // Reserve stack pointer
  Reserved.set(getStackRegister(MF));

  // Reserve link register (return address)
  Reserved.set(SPEX::LR);

  // Reserve accumulator register (implicit operand in many ops)
  Reserved.set(SPEX::RX);

  return Reserved;
}

const MCPhysReg *
SPEXRegisterInfo::getCalleeSavedRegs(const MachineFunction *) const {
  return CSR_SPEX_SaveList;
}

const uint32_t *
SPEXRegisterInfo::getCallPreservedMask(const MachineFunction &,
                                         CallingConv::ID) const {
  return CSR_SPEX_RegMask;
}

bool SPEXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
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
