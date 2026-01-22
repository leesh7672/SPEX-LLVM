#include "SPEX64InstrInfo.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SPEX64GenInstrInfo.inc"

using namespace llvm;

SPEX64InstrInfo::SPEX64InstrInfo(const TargetSubtargetInfo &STI)
    : SPEX64GenInstrInfo(STI, RI) {}
