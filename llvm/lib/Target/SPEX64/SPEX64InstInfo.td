def NOP : Instruction {
  let OutOperandList = (outs);
  let InOperandList  = (ins);
  let AsmString = "nop";
}
