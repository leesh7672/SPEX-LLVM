add_llvm_library(LLVMSPEXInfo
  SPEXTargetInfo.cpp

  LINK_COMPONENTS
  Support
  TargetParser
)
