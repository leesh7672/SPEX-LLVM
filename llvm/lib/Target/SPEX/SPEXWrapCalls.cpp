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
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {

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

        // Insert after call.
        auto After = std::next(It);
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
