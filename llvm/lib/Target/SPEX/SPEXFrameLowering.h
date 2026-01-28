//===-- SPEXFrameLowering.h - Define frame lowering for SPEX -*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXFRAMELOWERING_H
#define LLVM_LIB_TARGET_SPEX_SPEXFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class SPEXSubtarget;

class SPEXFrameLowering : public TargetFrameLowering {
public:
  explicit SPEXFrameLowering();

  // Whether the function needs a dedicated frame pointer.
  bool hasFPImpl(const MachineFunction &MF) const override;

  // Emit prologue/epilogue: currently no-op (no stack frame yet).
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  // Tell LLVM we don't create/need a reserved call frame right now.
  bool hasReservedCallFrame(const MachineFunction &MF) const override;

  // Remove ADJCALLSTACKDOWN/UP pseudos unconditionally.
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  // If you later add callee-saved regs / stack, expand these.
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SPEX_SPEXFRAMELOWERING_H
