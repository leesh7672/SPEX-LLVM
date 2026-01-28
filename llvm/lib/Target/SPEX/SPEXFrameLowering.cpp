//===-- SPEXFrameLowering.cpp - SPEX frame lowering -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXFrameLowering.h"

#include "SPEX.h"
#include "SPEXInstrInfo.h"
#include "SPEXSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

SPEXFrameLowering::SPEXFrameLowering()
    // Choose a reasonable default: Stack grows down, 16-byte alignment.
    // Even if you don't use a stack yet, LLVM expects a consistent policy.
    : TargetFrameLowering(StackGrowsDown, Align(16),
                          /*LocalAreaOffset=*/0,
                          /*TransientStackAlignment=*/Align(16)) {}

bool SPEXFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  return MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken() ||
         MFI.hasStackMap() || MFI.hasPatchPoint() ||
         MFI.hasOpaqueSPAdjustment();
}

void SPEXFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  // No-op for now.
  // Later: adjust SP, spill callee-saved regs, set FP, etc.
  (void)MF;
  (void)MBB;
}

void SPEXFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  // No-op for now.
  // Later: restore callee-saved regs, restore SP, etc.
  (void)MF;
  (void)MBB;
}

bool SPEXFrameLowering::hasReservedCallFrame(
    const MachineFunction &MF) const {
  // If you don't model call frames (no stack args), return false so LLVM
  // won't try to reserve fixed call frame space.
  (void)MF;
  return false;
}

MachineBasicBlock::iterator SPEXFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // LLVM inserts ADJCALLSTACKDOWN / ADJCALLSTACKUP around calls.
  // For targets that don't use a stack (yet), just delete them.
  (void)MF;
  return MBB.erase(I);
}

void SPEXFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  // Minimal bring-up: no callee-saved regs are spilled.
  // If you later define CSR sets and want spills, mark them here.
  (void)MF;
  (void)SavedRegs;
  (void)RS;
}
