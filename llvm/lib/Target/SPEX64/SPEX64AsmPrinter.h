#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ASMPRINTER_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ASMPRINTER_H

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class SPEX64AsmPrinter final : public AsmPrinter {
public:
  SPEX64AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}
  ~SPEX64AsmPrinter();

  StringRef getPassName() const override { return "SPEX64 Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
};

} // namespace llvm

#endif
