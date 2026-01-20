#include "SPEX64TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheSPEX64Target() {
  static Target TheSPEX64Target;
  return TheSPEX64Target;
}

extern "C" void LLVMInitializeSPEX64TargetInfo() {
  RegisterTarget<Triple::spex64, /*HasJIT=*/false> X(getTheSPEX64Target(),
                                                     "spex64", "SPEX64", "SPEX64");
}
