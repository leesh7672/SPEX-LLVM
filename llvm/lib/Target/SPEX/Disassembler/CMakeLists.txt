add_llvm_component_library(LLVMSPEXDisassembler
  SPEXDisassembler.cpp

  LINK_COMPONENTS
  CodeGenTypes
  SPEXDesc
  SPEXInfo
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  SPEX
  )

target_include_directories(LLVMSPEXDisassembler PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/..
)
