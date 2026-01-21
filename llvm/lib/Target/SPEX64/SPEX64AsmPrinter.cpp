#include "SPEX64AsmPrinter.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

void SPEX64AsmPrinter::emitInstruction(const MachineInstr *MI) {
  report_fatal_error(
      "SPEX64AsmPrinter: instruction lowering not implemented yet");
}

SPEX64AsmPrinter::~SPEX64AsmPrinter() = default;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPEX64AsmPrinter() {
  RegisterAsmPrinter<SPEX64AsmPrinter>(getTheSPEX64Target());
}
