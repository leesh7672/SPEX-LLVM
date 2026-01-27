//===-- SPEX64WrapCalls.cpp - Wrap call sites with lane sync --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass implements the SPEX64 ABI rule for compiler-generated function
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

#include "SPEX64.h"
#include "SPEX64InstrInfo.h"
#include "SPEX64Subtarget.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {

class SPEX64WrapCalls final : public MachineFunctionPass {
public:
  static char ID;
  SPEX64WrapCalls() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "SPEX64 Wrap Calls (lstop/lwait/lwake)";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const auto &STI = MF.getSubtarget<SPEX64Subtarget>();
    const auto &TII = static_cast<const SPEX64InstrInfo &>(*STI.getInstrInfo());

    bool Changed = false;

    for (auto &MBB : MF) {
      for (auto It = MBB.begin(), End = MBB.end(); It != End; ++It) {
        MachineInstr &MI = *It;

        // Prefer opcode checks because some calls may be pseudos pre-expansion.
        const unsigned Opc = MI.getOpcode();
        const bool IsSPEX64Call =
            MI.isCall() || Opc == SPEX64::CALL || Opc == SPEX64::CALLR ||
            Opc == SPEX64::CALLI || Opc == SPEX64::CALL32 || Opc == SPEX64::CALL64;

        if (!IsSPEX64Call)
          continue;

        const DebugLoc DL = MI.getDebugLoc();

        // Insert before call.
        BuildMI(MBB, It, DL, TII.get(SPEX64::LSTOP));
        BuildMI(MBB, It, DL, TII.get(SPEX64::LWAIT));

        // Insert after call. We keep this immediately after the call (or call
        // pseudo). If later we want to guarantee "after return-value copies",
        // we can move this past consecutive COPYs, but for now this is the
        // minimal correct semantic fence for lane state.
        auto After = std::next(It);
        BuildMI(MBB, After, DL, TII.get(SPEX64::LWAKE));

        Changed = true;
      }
    }

    return Changed;
  }
};

} // end anonymous namespace

char SPEX64WrapCalls::ID = 0;

MachineFunctionPass *llvm::createSPEX64WrapCallsPass() {
  return new SPEX64WrapCalls();
}
