#include "SPEX64ISelDAGToDAG.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

FunctionPass *llvm::createSPEX64ISelDag(SPEX64TargetMachine &) {
  report_fatal_error("SPEX64 ISel not implemented yet");
}
