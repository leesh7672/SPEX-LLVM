add_llvm_component_library(LLVMSPEXAsmParser
  SPEXAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  SPEXDesc
  Support

  ADD_TO_COMPONENT
  SPEX
)

add_dependencies(LLVMSPEXAsmParser SPEXCommonTableGen)

target_include_directories(LLVMSPEXAsmParser PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
