add_llvm_component_library(LLVMSPEX64Disassembler
  SPEX64Disassembler.cpp

  LINK_COMPONENTS
  CodeGenTypes
  SPEX64Desc
  SPEX64Info
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  SPEX64
  )

target_include_directories(LLVMSPEX64Disassembler PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/..
)
