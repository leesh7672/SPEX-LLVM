#include "SPEX64ISelLowering.h"
#include "SPEX64Subtarget.h"
#include "SPEX64TargetMachine.h"

using namespace llvm;

SPEX64TargetLowering::SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                           const SPEX64Subtarget &ST)
    : TargetLowering(TM), ST(ST) {

  addRegisterClass(MVT::i64, &SPEX64::GPR);

  setBooleanContents(ZeroOrOneBooleanContent);

  // stack alignment
  setStackPointerRegisterToSaveRestore(ST.getInstrInfo()->getRegisterInfo().getStackRegister(/*MF*/ ???));
}

const char *SPEX64TargetLowering::getTargetNodeName(unsigned Opcode) const {
  return nullptr;
}
