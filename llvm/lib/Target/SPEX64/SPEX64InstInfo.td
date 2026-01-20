include "SPEX64RegisterInfo.td"

class SPEX64Instr<string asmstr, dag outs, dag ins> : Instruction {
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString      = asmstr;

  let Size = 4;
  bits<32> Inst;
}

def SPEX64InstrInfo : InstrInfo;

def NOP : SPEX64Instr<"nop", (outs), (ins)>;

defm : RemapAllTargetPseudoPointerOperands<GPR>;
