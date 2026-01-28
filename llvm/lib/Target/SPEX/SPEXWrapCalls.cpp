//===-- SPEXWrapCalls.cpp - Wrap call sites with lane sync --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass implements the SPEX ABI rule for compiler-generated function
// calls:
//
//   lstop; lwait; call ...; lwake
//
// Semantics (per ISA spec):
// - lstop/lwait/lwake implicitly target all lanes in the same core except the
//   executing lane (self).
// - This ensures the privileged scalar-call/ret semantics (SCF + full lane
//   state restore on ret) is safe for general C calls.
//
// IMPORTANT:
// We must not implement this in SelectionDAG lowering, because chain nodes can
// be hoisted to function entry and/or survive without an actual call in the
// final DAG, leading to invalid MachineInstr emission. Therefore we insert the
// wrapper at the MachineInstr level, strictly around call instructions.
//===----------------------------------------------------------------------===//

#include "SPEX.h"
#include "SPEXInstrInfo.h"
#include "SPEXSubtarget.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {

static void insertSaveAllGPRs(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator InsertPt,
                              const DebugLoc &DL, const SPEXInstrInfo &TII) {
  // NOTE:
  // The current RTL model uses a core-wide GPR file, so if other lanes are
  // running then regular calling convention assumptions do not hold. As a
  // conservative correctness hack, we save (almost) all GPRs around each call
  // while lanes are stopped (lstop/lwait).
  //
  // We intentionally do NOT restore R0 so call return values remain visible.
  // We also do not save/restore R63 (SP) because we adjust it around the
  // save area itself.
  Register SP = SPEX::R63;

  SmallVector<Register, 64> RegsToSave;
  for (MCPhysReg R : SPEX::GPRRegClass.getRegisters()) {
    // Don't save the return value register (R0) or stack pointer (R63).
    if (R == SPEX::R0 || R == SPEX::R63)
      continue;
    RegsToSave.push_back(R);
  }

  const int64_t FrameSize = int64_t(RegsToSave.size()) * 8; // 16-byte aligned

  // SP -= FrameSize
  BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_SUB64ri), SP)
      .addReg(SP)
      .addImm(FrameSize);

  int64_t Off = 0;
  for (Register R : RegsToSave) {
    BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_ST64))
        .addReg(R)
        .addReg(SP)
        .addImm(Off);
    Off += 8;
  }
}

static void insertRestoreAllGPRs(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator InsertPt,
                                 const DebugLoc &DL,
                                 const SPEXInstrInfo &TII) {
  Register SP = SPEX::R63;

  SmallVector<Register, 64> RegsToRestore;
  for (MCPhysReg R : SPEX::GPRRegClass.getRegisters()) {
    if (R == SPEX::R0 || R == SPEX::R63)
      continue;
    RegsToRestore.push_back(R);
  }

  const int64_t FrameSize = int64_t(RegsToRestore.size()) * 8;

  int64_t Off = 0;
  for (Register R : RegsToRestore) {
    BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_LDZ64), R)
        .addReg(SP)
        .addImm(Off);
    Off += 8;
  }

  // SP += FrameSize
  BuildMI(MBB, InsertPt, DL, TII.get(SPEX::PSEUDO_ADD64ri), SP)
      .addReg(SP)
      .addImm(FrameSize);
}

class SPEXWrapCalls final : public MachineFunctionPass {
public:
  static char ID;
  SPEXWrapCalls() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "SPEX Wrap Calls (lstop/lwait/lwake)";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const auto &STI = MF.getSubtarget<SPEXSubtarget>();
    const auto &TII = static_cast<const SPEXInstrInfo &>(*STI.getInstrInfo());

    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto It = MBB.begin(), End = MBB.end(); It != End; ++It) {
        MachineInstr &MI = *It;

        // Prefer opcode checks because some calls may be pseudos pre-expansion.
        const unsigned Opc = MI.getOpcode();
        const bool IsSPEXCall = MI.isCall() || Opc == SPEX::PSEUDO_CALL ||
                                Opc == SPEX::PSEUDO_CALLR ||
                                Opc == SPEX::CALL || Opc == SPEX::CALL32 ||
                                Opc == SPEX::CALL64;

        if (!IsSPEXCall)
          continue;

        const DebugLoc DL = MI.getDebugLoc();

        // Insert before call.
        BuildMI(MBB, It, DL, TII.get(SPEX::LSTOP));
        BuildMI(MBB, It, DL, TII.get(SPEX::LWAIT));
        // Only touch the shared stack pointer *after* lanes are stopped.
        insertSaveAllGPRs(MBB, It, DL, TII);

        // Insert after call: restore regs/SP before waking other lanes.
        auto After = std::next(It);
        insertRestoreAllGPRs(MBB, After, DL, TII);
        BuildMI(MBB, After, DL, TII.get(SPEX::LWAKE));

        Changed = true;
      }
    }

    return Changed;
  }
};

} // end anonymous namespace

char SPEXWrapCalls::ID = 0;

MachineFunctionPass *llvm::createSPEXWrapCallsPass() {
  return new SPEXWrapCalls();
}
