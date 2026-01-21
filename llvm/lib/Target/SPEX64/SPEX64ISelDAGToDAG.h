#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ISELDAGTODAG_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ISELDAGTODAG_H

namespace llvm {
class SPEX64TargetMachine;
class FunctionPass;
FunctionPass *createSPEX64ISelDag(SPEX64TargetMachine &TM);
} // namespace llvm

#endif
