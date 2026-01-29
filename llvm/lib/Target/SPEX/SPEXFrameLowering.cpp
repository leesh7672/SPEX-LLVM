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
#include "llvm/CodeGen/MachineBasicBlock.h"
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
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();

  // Maintain the target's declared stack alignment.
  if (StackSize)
    StackSize = alignTo(StackSize, 16);
  MFI.setStackSize(StackSize);

  if (!StackSize)
    return;

  const SPEXSubtarget &ST = MF.getSubtarget<SPEXSubtarget>();
  const auto &TII = static_cast<const SPEXInstrInfo &>(*ST.getInstrInfo());
  const auto &TRI =
      static_cast<const SPEXRegisterInfo &>(*ST.getRegisterInfo());
  Register SP = TRI.getStackRegister(MF);

  // Insert before everything in the entry block (including spills inserted by
  // PEI) so all frame references see the adjusted SP.
  MachineBasicBlock::iterator InsertPt = MBB.begin();
  DebugLoc DL;

  // SP -= StackSize
  auto MI = BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_SUB64ri), SP)
                .addReg(SP)
                .addImm(StackSize);
  MI->setFlag(MachineInstr::FrameSetup);
}

void SPEXFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  if (!StackSize)
    return;

  const SPEXSubtarget &ST = MF.getSubtarget<SPEXSubtarget>();
  const auto &TII = static_cast<const SPEXInstrInfo &>(*ST.getInstrInfo());
  const auto &TRI =
      static_cast<const SPEXRegisterInfo &>(*ST.getRegisterInfo());
  Register SP = TRI.getStackRegister(MF);

  // Insert immediately before the first terminator so restores still use the
  // current (decremented) SP.
  MachineBasicBlock::iterator InsertPt = MBB.getFirstTerminator();
  DebugLoc DL;

  // SP += StackSize
  auto MI = BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_ADD64ri), SP)
                .addReg(SP)
                .addImm(StackSize);
  MI->setFlag(MachineInstr::FrameDestroy);
}

bool SPEXFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
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
  // Use the generic logic driven by CSR_SPEX (TableGen) and the register
  // allocator's decisions.
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
}
