include "SPEX64RegisterInfo.td"

def SPEX64Imm9  : Operand<i32>;
def SPEX64Imm32 : Operand<i32>;
def SPEX64Imm64 : Operand<i64>;

class SPEX64Inst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                 bits<2> sz2, bits<1> i1, bits<1> i64> : Instruction {
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString      = asmstr;

  let Size = 4;
  bits<32> Inst;
  bits<6> r6;
  bits<9> K9;

  let Inst{31-20} = opc;
  let Inst{19}    = x1;
  let Inst{18-17} = sz2;
  let Inst{16}    = i1;
  let Inst{15}    = i64;
  let Inst{14-9}  = r6;
  let Inst{8-0}   = K9;

  let r6 = 0;
  let K9 = 0;
}

class SPEX64K9Inst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                   bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEX64Inst<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  bits<9> k9;
  let K9 = k9;
}

def SPEX64InstrInfo : InstrInfo;

// SYS opcodes (0x000-0x00B)
def NOP     : SPEX64Inst<"nop",     (outs), (ins), 0x000, 0, 0b00, 0, 0>;
def HLT     : SPEX64Inst<"hlt",     (outs), (ins), 0x001, 0, 0b00, 0, 0>;
def RET     : SPEX64Inst<"ret",     (outs), (ins), 0x002, 0, 0b00, 0, 0>;
def SYSCALL : SPEX64Inst<"syscall", (outs), (ins), 0x003, 0, 0b00, 0, 0>;
def TRAP    : SPEX64Inst<"trap",    (outs), (ins), 0x004, 0, 0b00, 0, 0>;
def ERET    : SPEX64Inst<"eret",    (outs), (ins), 0x005, 0, 0b00, 0, 0>;
def FENCE   : SPEX64Inst<"fence",   (outs), (ins), 0x006, 0, 0b00, 0, 0>;
def PAUSE   : SPEX64Inst<"pause",   (outs), (ins), 0x007, 0, 0b00, 0, 0>;
def INTEN   : SPEX64Inst<"inten",   (outs), (ins), 0x008, 0, 0b00, 0, 0>;
def INTDIS  : SPEX64Inst<"intdis",  (outs), (ins), 0x009, 0, 0b00, 0, 0>;
def WFI     : SPEX64Inst<"wfi",     (outs), (ins), 0x00A, 0, 0b00, 0, 0>;
def ILL     : SPEX64Inst<"ill",     (outs), (ins), 0x00B, 0, 0b00, 0, 0>;

// bcc (absolute target, K9 condition)
def BCC32 : SPEX64K9Inst<"bcc\t$k9, $target", (outs),
                         (ins SPEX64Imm9:$k9, SPEX64Imm32:$target),
                         0x100, 0, 0b00, 1, 0> {
  let Size = 8;
}
def BCC64 : SPEX64K9Inst<"bcc\t$k9, $target", (outs),
                         (ins SPEX64Imm9:$k9, SPEX64Imm64:$target),
                         0x100, 0, 0b00, 1, 1> {
  let Size = 12;
}

// jmp / call / sp
def JMP_R : SPEX64Inst<"jmp\t$r6", (outs), (ins GPR:$r6), 0x200, 0, 0b00, 0, 0>;
def JMP32 : SPEX64Inst<"jmp\t$target", (outs), (ins SPEX64Imm32:$target),
                       0x200, 0, 0b00, 1, 0> {
  let Size = 8;
}
def JMP64 : SPEX64Inst<"jmp\t$target", (outs), (ins SPEX64Imm64:$target),
                       0x200, 0, 0b00, 1, 1> {
  let Size = 12;
}

def CALL_R : SPEX64Inst<"call\t$r6", (outs), (ins GPR:$r6), 0x203, 0, 0b00, 0, 0>;
def CALL32 : SPEX64Inst<"call\t$target", (outs), (ins SPEX64Imm32:$target),
                        0x203, 0, 0b00, 1, 0> {
  let Size = 8;
}
def CALL64 : SPEX64Inst<"call\t$target", (outs), (ins SPEX64Imm64:$target),
                        0x203, 0, 0b00, 1, 1> {
  let Size = 12;
}

def SP_RX : SPEX64Inst<"sp\t$r6", (outs), (ins GPR:$r6), 0x206, 0, 0b00, 0, 0>;
def SP32 : SPEX64Inst<"sp\t$r6, $target", (outs),
                      (ins GPR:$r6, SPEX64Imm32:$target),
                      0x206, 0, 0b00, 1, 0> {
  let Size = 8;
}
def SP64 : SPEX64Inst<"sp\t$r6, $target", (outs),
                      (ins GPR:$r6, SPEX64Imm64:$target),
                      0x206, 0, 0b00, 1, 1> {
  let Size = 12;
}

// mov* (rx <-> r6). Lane-local variants are represented as separate mnemonics.
multiclass SPEX64_MOV_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def MOV#def_suf : SPEX64Inst<"mov"#asm_suf#"\t$r6", (outs), (ins GPR:$r6),
                               0x300, 0, sz2, 0, 0>;
  def MOV#def_suf#"_R" : SPEX64Inst<"mov"#asm_suf#"\t$r6, rx", (outs),
                                   (ins GPR:$r6),
                                   0x300, 1, sz2, 0, 0>;
  def MOV#def_suf#"_CX0" : SPEX64Inst<"mov"#asm_suf#"\tcx0", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1" : SPEX64Inst<"mov"#asm_suf#"\tcx1", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS" : SPEX64Inst<"mov"#asm_suf#"\tcas", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 3;
  }
  def MOV#def_suf#"_CX0_R" : SPEX64Inst<"mov"#asm_suf#"\tcx0, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1_R" : SPEX64Inst<"mov"#asm_suf#"\tcx1, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS_R" : SPEX64Inst<"mov"#asm_suf#"\tcas, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 3;
  }
}
defm MOV : SPEX64_MOV_SZ<"B", ".b", 0b00>;
defm MOV : SPEX64_MOV_SZ<"H", ".h", 0b01>;
defm MOV : SPEX64_MOV_SZ<"W", ".w", 0b10>;
defm MOV : SPEX64_MOV_SZ<"D", ".d", 0b11>;

// li*
multiclass SPEX64_LI_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def LI#def_suf#"_32" : SPEX64Inst<"li"#asm_suf#"\t$imm", (outs),
                                  (ins SPEX64Imm32:$imm),
                                  0x302, 0, sz2, 1, 0> {
    let Size = 8;
  }
  def LI#def_suf#"_64" : SPEX64Inst<"li"#asm_suf#"\t$imm", (outs),
                                  (ins SPEX64Imm64:$imm),
                                  0x302, 0, sz2, 1, 1> {
    let Size = 12;
  }
}
defm LI : SPEX64_LI_SZ<"B", ".b", 0b00>;
defm LI : SPEX64_LI_SZ<"H", ".h", 0b01>;
defm LI : SPEX64_LI_SZ<"W", ".w", 0b10>;
defm LI : SPEX64_LI_SZ<"D", ".d", 0b11>;

// Binary ALU (reg + imm32 + imm64)
multiclass SPEX64_ALU_SZ<bits<12> opc, string mn, string def_suf,
                         string asm_suf, bits<2> sz2> {
  def NAME#def_suf#"_R" : SPEX64Inst<mn#asm_suf#"\t$r6", (outs),
                                    (ins GPR:$r6),
                                    opc, 0, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64Inst<mn#asm_suf#"\t$imm", (outs),
                                      (ins SPEX64Imm32:$imm),
                                      opc, 0, sz2, 1, 0> {
    let Size = 8;
  }
  def NAME#def_suf#"_I64" : SPEX64Inst<mn#asm_suf#"\t$imm", (outs),
                                      (ins SPEX64Imm64:$imm),
                                      opc, 0, sz2, 1, 1> {
    let Size = 12;
  }
}
multiclass SPEX64_ALU<bits<12> opc, string mn> {
  defm NAME : SPEX64_ALU_SZ<opc, mn, "B", ".b", 0b00>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "H", ".h", 0b01>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "W", ".w", 0b10>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "D", ".d", 0b11>;
}
defm ADD  : SPEX64_ALU<0x400, "add">;
defm SUB  : SPEX64_ALU<0x401, "sub">;
defm AND  : SPEX64_ALU<0x402, "and">;
defm OR   : SPEX64_ALU<0x403, "or">;
defm XOR  : SPEX64_ALU<0x404, "xor">;
defm CMP  : SPEX64_ALU<0x405, "cmp">;
defm TEST : SPEX64_ALU<0x406, "test">;

// Unary ALU
multiclass SPEX64_UNARY_SZ<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf, (outs), (ins),
                               opc, 0, sz2, 0, 0>;
}
multiclass SPEX64_UNARY<bits<12> opc, string mn> {
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "B", ".b", 0b00>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "H", ".h", 0b01>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "W", ".w", 0b10>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "D", ".d", 0b11>;
}
defm NOT : SPEX64_UNARY<0x410, "not">;
defm NEG : SPEX64_UNARY<0x411, "neg">;
defm INC : SPEX64_UNARY<0x412, "inc">;
defm DEC : SPEX64_UNARY<0x413, "dec">;

// Shift / bit (imm32 only)
multiclass SPEX64_SHIFT_SZ<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf#"\t$imm", (outs),
                                (ins SPEX64Imm32:$imm),
                                opc, 0, sz2, 1, 0> {
    let Size = 8;
  }
}
multiclass SPEX64_SHIFT<bits<12> opc, string mn> {
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "B", ".b", 0b00>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "H", ".h", 0b01>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "W", ".w", 0b10>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "D", ".d", 0b11>;
}
defm SHL  : SPEX64_SHIFT<0x600, "shl">;
defm SHR  : SPEX64_SHIFT<0x601, "shr">;
defm SAR  : SPEX64_SHIFT<0x602, "sar">;
defm BTST : SPEX64_SHIFT<0x610, "btst">;
defm BSET : SPEX64_SHIFT<0x611, "bset">;
defm BCLR : SPEX64_SHIFT<0x612, "bclr">;
defm BINV : SPEX64_SHIFT<0x613, "binv">;

// Load / store (base rx, optional offset)
multiclass SPEX64_LS_NO_R6<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf#"\t[rx]", (outs), (ins),
                               opc, x1, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64Inst<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEX64Imm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Size = 8;
  }
  def NAME#def_suf#"_I64" : SPEX64Inst<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEX64Imm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Size = 12;
  }
}
multiclass SPEX64_LS_R6<bits<12> opc, string mn, string def_suf, string asm_suf,
                        bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf#"\t$r6, [rx]", (outs),
                                (ins GPR:$r6), opc, x1, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64Inst<mn#asm_suf#"\t$r6, [rx + $imm]", (outs),
                                      (ins GPR:$r6, SPEX64Imm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Size = 8;
  }
  def NAME#def_suf#"_I64" : SPEX64Inst<mn#asm_suf#"\t$r6, [rx + $imm]", (outs),
                                      (ins GPR:$r6, SPEX64Imm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Size = 12;
  }
}
multiclass SPEX64_LS_NO_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "B", ".b", 0b00, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "H", ".h", 0b01, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "W", ".w", 0b10, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "D", ".d", 0b11, x1>;
}
multiclass SPEX64_LS_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX64_LS_R6<opc, mn, "B", ".b", 0b00, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "H", ".h", 0b01, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "W", ".w", 0b10, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "D", ".d", 0b11, x1>;
}

defm LDZ : SPEX64_LS_NO_R6_SZS<0x700, "ldz", 0>;
defm LDZ_R : SPEX64_LS_R6_SZS<0x700, "ldz.r", 1>;
defm LDS : SPEX64_LS_NO_R6_SZS<0x701, "lds", 0>;
defm LDS_R : SPEX64_LS_R6_SZS<0x701, "lds.r", 1>;
defm ST : SPEX64_LS_NO_R6_SZS<0x702, "st", 0>;
defm ST_R : SPEX64_LS_R6_SZS<0x702, "st.r", 1>;

// Atomic
def CAS64 : SPEX64Inst<"cas64\t$r6", (outs), (ins GPR:$r6), 0x800, 0, 0b11, 0, 0>;

// CSR / MMU / IPI
def CSRR : SPEX64Inst<"csrr\t$imm", (outs), (ins SPEX64Imm32:$imm),
                      0x900, 0, 0b00, 1, 0> {
  let Size = 8;
}
def CSRW : SPEX64Inst<"csrw\t$imm", (outs), (ins SPEX64Imm32:$imm),
                      0x902, 0, 0b00, 1, 0> {
  let Size = 8;
}
def TLBFLUSH : SPEX64Inst<"tlbflush", (outs), (ins), 0x910, 0, 0b00, 0, 0>;
def TLBFLUSHASID : SPEX64Inst<"tlbflushasid\t$r6", (outs), (ins GPR:$r6),
                              0x911, 0, 0b00, 0, 0>;
def ASIDSET : SPEX64Inst<"asidset\t$r6", (outs), (ins GPR:$r6),
                         0x912, 0, 0b00, 0, 0>;
def IPISEND : SPEX64Inst<"ipisend\t$r6", (outs), (ins GPR:$r6),
                         0x920, 0, 0b00, 0, 0>;

// Lane read (K9 encodes LID and FIELD).
def LRD : SPEX64K9Inst<"lrd\t$k9", (outs), (ins SPEX64Imm9:$k9),
                       0xA00, 0, 0b00, 0, 0>;

defm : RemapAllTargetPseudoPointerOperands<GPR>;
