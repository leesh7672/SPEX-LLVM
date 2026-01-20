add_llvm_component_group(SPEX64)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

add_llvm_target(SPEX64CodeGen
  SPEX64TargetMachine.cpp
  SPEX64TargetMachineImpl.cpp
)

target_link_libraries(LLVMSPEX64CodeGen PRIVATE LLVMSPEX64Info LLVMSPEX64Desc)
