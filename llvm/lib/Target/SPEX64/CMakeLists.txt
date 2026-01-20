set(LLVM_TARGET_DEFINITIONS SPEX64.td)

tablegen(LLVM SPEX64GenInstrInfo.inc       -gen-instr-info)
tablegen(LLVM SPEX64GenRegisterInfo.inc    -gen-register-info)
tablegen(LLVM SPEX64GenSubtargetInfo.inc   -gen-subtarget)
#tablegen(LLVM SPEX64GenAsmMatcher.inc      -gen-asm-matcher)
#tablegen(LLVM SPEX64GenAsmWriter.inc       -gen-asm-writer)

add_public_tablegen_target(SPEX64CommonTableGen)
add_llvm_component_group(SPEX64)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

add_llvm_target(SPEX64CodeGen
  SPEX64TargetMachine.cpp
  SPEX64TargetMachineImpl.cpp
)

target_link_libraries(LLVMSPEX64CodeGen PRIVATE LLVMSPEX64Info LLVMSPEX64Desc)
target_include_directories(LLVMSPEX64Desc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
