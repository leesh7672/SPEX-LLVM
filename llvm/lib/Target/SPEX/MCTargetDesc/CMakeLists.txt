add_llvm_library(LLVMSPEXDesc
  SPEXMCTargetDesc.cpp
  SPEXMCCodeEmitter.cpp
  SPEXAsmBackend.cpp
  SPEXELFObjectWriter.cpp
  SPEXMCAsmInfo.cpp
  SPEXInstPrinter.cpp

  DEPENDS
  SPEXCommonTableGen

  LINK_COMPONENTS
  MC
  Support
  TargetParser
)
