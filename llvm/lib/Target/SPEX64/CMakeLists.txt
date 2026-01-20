add_llvm_target(SPEX64CodeGen
  SPEX64TargetMachine.cpp
)
add_custom_target(SPEX64 DEPENDS LLVMSPEX64CodeGen)