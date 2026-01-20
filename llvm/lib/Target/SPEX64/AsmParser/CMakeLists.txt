add_llvm_component_library(LLVMSPEX64AsmParser
  SPEX64AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  SPEX64Desc
  Support

  ADD_TO_COMPONENT
  SPEX64
)

add_dependencies(LLVMSPEX64AsmParser SPEX64CommonTableGen)

target_include_directories(LLVMSPEX64AsmParser PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
