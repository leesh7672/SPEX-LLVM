#include "SPEX64InstrInfo.h"
using namespace llvm;

SPEX64InstrInfo::SPEX64InstrInfo(const TargetSubtargetInfo &STI)
    : SPEX64GenInstrInfo(STI, RI) {}
