#include "SPEXTargetObjectFile.h"

#include "llvm/MC/MCContext.h"

using namespace llvm;

void SPEXTargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
}

MCSection *SPEXTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  if (Kind.isMergeableCString())
    return TargetLoweringObjectFileELF::SelectSectionForGlobal(
        GO, SectionKind::getData(), TM);

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}
