#include "SPEX.h"
#include "SPEXInstrInfo.h"
#include "SPEXSubtarget.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {
class SPEXExpandPseudo final : public MachineFunctionPass {
public:
  static char ID;
  SPEXExpandPseudo() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "SPEX Expand Post-RA Pseudos";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const auto &STI = MF.getSubtarget<SPEXSubtarget>();
    const auto &TII = static_cast<const SPEXInstrInfo &>(*STI.getInstrInfo());

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

char SPEXExpandPseudo::ID = 0;

MachineFunctionPass *llvm::createSPEXExpandPseudoPass() {
  return new SPEXExpandPseudo();
}
