#ifndef LLVM_LIB_TARGET_SPEX_SPEXTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_SPEX_SPEXTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

/// SPEXTargetObjectFile - ELF object file lowering for SPEX.
class SPEXTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  ~SPEXTargetObjectFile() override = default;
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SPEX_SPEXTARGETOBJECTFILE_H
