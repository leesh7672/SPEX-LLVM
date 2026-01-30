set(LLVM_TARGET_DEFINITIONS SPEX.td)

tablegen(LLVM SPEXGenInstrInfo.inc       -gen-instr-info)
tablegen(LLVM SPEXGenDAGISel.inc         -gen-dag-isel)
tablegen(LLVM SPEXGenMCCodeEmitter.inc  -gen-emitter)
tablegen(LLVM SPEXGenRegisterInfo.inc    -gen-register-info)
tablegen(LLVM SPEXGenSubtargetInfo.inc   -gen-subtarget)
tablegen(LLVM SPEXGenAsmMatcher.inc      -gen-asm-matcher)
tablegen(LLVM SPEXGenAsmWriter.inc       -gen-asm-writer)
tablegen(LLVM SPEXGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM SPEXCallingConv.inc -gen-callingconv)

add_public_tablegen_target(SPEXCommonTableGen)
add_llvm_component_group(SPEX)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)
add_subdirectory(Disassembler)

add_llvm_target(SPEXCodeGen
  SPEXAsmPrinter.cpp
  SPEXMCInstLower.cpp
  SPEXTargetMachine.cpp
  SPEXSelectionDAGInfo.cpp
  SPEXTargetMachineImpl.cpp
  SPEXTargetObjectFile.cpp
  SPEXSubtarget.cpp
  SPEXInstrInfo.cpp
  SPEXExpandPseudo.cpp
  SPEXWrapCalls.cpp
  SPEXRegisterInfo.cpp
  SPEXFrameLowering.cpp
  SPEXISelDAGToDAG.cpp
  SPEXISelLowering.cpp

  LINK_COMPONENTS
    Analysis
    AsmPrinter
    CodeGen
    CodeGenTypes
    Core
    MC
    SelectionDAG
    Support
    Target
    TargetParser

  ADD_TO_COMPONENT
    SPEX
)

target_link_libraries(LLVMSPEXCodeGen PRIVATE LLVMSPEXInfo LLVMSPEXDesc LLVMSPEXAsmParser)
target_include_directories(LLVMSPEXCodeGen PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(LLVMSPEXDesc   PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
