set(LLVM_TARGET_DEFINITIONS SPEX64.td)

tablegen(LLVM SPEX64GenInstrInfo.inc       -gen-instr-info)
tablegen(LLVM SPEX64GenMCCodeEmitter.inc  -gen-emitter)
tablegen(LLVM SPEX64GenRegisterInfo.inc    -gen-register-info)
tablegen(LLVM SPEX64GenSubtargetInfo.inc   -gen-subtarget)
tablegen(LLVM SPEX64GenAsmMatcher.inc      -gen-asm-matcher)
tablegen(LLVM SPEX64GenAsmWriter.inc       -gen-asm-writer)
tablegen(LLVM SPEX64GenDisassemblerTables.inc -gen-disassembler)

add_public_tablegen_target(SPEX64CommonTableGen)
add_llvm_component_group(SPEX64)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)
add_subdirectory(Disassembler)

add_llvm_target(SPEX64CodeGen
  SPEX64AsmPrinter.cpp
  SPEX64TargetMachine.cpp
  SPEX64TargetMachineImpl.cpp
  SPEX64Subtarget.cpp
  SPEX64InstrInfo.cpp
  SPEX64RegisterInfo.cpp
  SPEX64FrameLowering.cpp
  SPEX64ISelDAGToDAG.cpp
  SPEX64ISelLowering.cpp
)

target_link_libraries(LLVMSPEX64CodeGen PRIVATE LLVMSPEX64Info LLVMSPEX64Desc LLVMSPEX64AsmParser)
target_include_directories(LLVMSPEX64CodeGen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(LLVMSPEX64Desc   PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

