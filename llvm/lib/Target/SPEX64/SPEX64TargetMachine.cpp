extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSPEX64Target() {
  RegisterTargetMachine<SPEX64TargetMachine> X(getTheSPEX64Target());
}
