#include "SPEX64.h"
#include "SPEX64InstrInfo.h"
#include "SPEX64Subtarget.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {
class SPEX64ExpandPseudo final : public MachineFunctionPass {
public:
  static char ID;
  SPEX64ExpandPseudo() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "SPEX64 Expand Post-RA Pseudos";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const auto &STI = MF.getSubtarget<SPEX64Subtarget>();
    const auto &TII = static_cast<const SPEX64InstrInfo &>(*STI.getInstrInfo());

    bool Changed = false;
    for (auto &MBB : MF) {
      for (auto I = MBB.begin(), E = MBB.end(); I != E;) {
        MachineInstr &MI = *I++;
        if (TII.expandPostRAPseudo(MI)) {
          Changed = true;
        }
      }
    }
    return Changed;
  }
};
} // end anonymous namespace

char SPEX64ExpandPseudo::ID = 0;

MachineFunctionPass *llvm::createSPEX64ExpandPseudoPass() {
  return new SPEX64ExpandPseudo();
}
