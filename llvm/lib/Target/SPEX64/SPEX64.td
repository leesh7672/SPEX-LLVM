include "llvm/Target/Target.td"

def SPEX64 : Target {
  let InstructionSet = SPEX64InstrInfo;
}
