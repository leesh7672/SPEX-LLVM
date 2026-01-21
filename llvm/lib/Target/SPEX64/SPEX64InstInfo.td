include "SPEX64RegisterInfo.td"

def SPEX64Imm9AsmOperand : AsmOperandClass {
  let Name = "SPEX64Imm9";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm9";
  let RenderMethod = "addImmOperands";
}
def SPEX64Imm32AsmOperand : AsmOperandClass {
  let Name = "SPEX64Imm32";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm32";
  let RenderMethod = "addImmOperands";
}
def SPEX64Imm64AsmOperand : AsmOperandClass {
  let Name = "SPEX64Imm64";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm64";
  let RenderMethod = "addImmOperands";
}

def SPEX64Imm9  : Operand<i32> {
  let ParserMatchClass = SPEX64Imm9AsmOperand;
  let DecoderMethod = "DecodeSPEX64Imm9";
}
def SPEX64Imm32 : Operand<i32> {
  let ParserMatchClass = SPEX64Imm32AsmOperand;
  let DecoderMethod = "DecodeSPEX64Imm32";
}
def SPEX64Imm64 : Operand<i64> {
  let ParserMatchClass = SPEX64Imm64AsmOperand;
  let DecoderMethod = "DecodeSPEX64Imm64";
}

class SPEX64Inst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                 bits<2> sz2, bits<1> i1, bits<1> i64> : Instruction {
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString      = asmstr;
  let Namespace      = "SPEX64";

  let Size = 4;
  bits<32> Inst;
  bits<6> r6;
  bits<9> K9;
  bits<64> imm;
  bits<64> target;

  let Inst{31-20} = opc;
  let Inst{19}    = x1;
  let Inst{18-17} = sz2;
  let Inst{16}    = i1;
  let Inst{15}    = i64;
  let Inst{14-9}  = r6;
  let Inst{8-0}   = K9;

  let K9 = 0;
  let imm = 0;
  let target = 0;
}

class SPEX64InstNoR6<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                     bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEX64Inst<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  let r6 = 0;
}

class SPEX64K9Inst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                   bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEX64Inst<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  bits<9> k9;
  let K9 = k9;
}

class SPEX64K9InstNoR6<string asmstr, dag outs, dag ins, bits<12> opc,
                       bits<1> x1, bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEX64InstNoR6<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  bits<9> k9;
  let K9 = k9;
}

class SPEX64BCCInst<string asmstr, dag outs, dag ins, bits<9> k9val,
                    bits<1> i64>
    : SPEX64K9InstNoR6<asmstr, outs, ins, 0x100, 0b0, 0b00, 1, i64> {
  let k9 = k9val;
}

def SPEX64InstrInfo : InstrInfo;

// SYS opcodes (0x000-0x00B)
def NOP     : SPEX64InstNoR6<"nop", (outs), (ins), 0x000, 0, 0b00, 0, 0>;
def HLT     : SPEX64InstNoR6<"hlt", (outs), (ins), 0x001, 0, 0b00, 0, 0>;
def RET     : SPEX64InstNoR6<"ret", (outs), (ins), 0x002, 0, 0b00, 0, 0>;
def SYSCALL : SPEX64InstNoR6<"syscall", (outs), (ins), 0x003, 0, 0b00, 0, 0>;
def TRAP    : SPEX64InstNoR6<"trap", (outs), (ins), 0x004, 0, 0b00, 0, 0>;
def ERET    : SPEX64InstNoR6<"eret", (outs), (ins), 0x005, 0, 0b00, 0, 0>;
def FENCE   : SPEX64InstNoR6<"fence", (outs), (ins), 0x006, 0, 0b00, 0, 0>;
def PAUSE   : SPEX64InstNoR6<"pause", (outs), (ins), 0x007, 0, 0b00, 0, 0>;
def INTEN   : SPEX64InstNoR6<"inten", (outs), (ins), 0x008, 0, 0b00, 0, 0>;
def INTDIS  : SPEX64InstNoR6<"intdis", (outs), (ins), 0x009, 0, 0b00, 0, 0>;
def WFI     : SPEX64InstNoR6<"wfi", (outs), (ins), 0x00A, 0, 0b00, 0, 0>;
def ILL     : SPEX64InstNoR6<"ill", (outs), (ins), 0x00B, 0, 0b00, 0, 0>;

// bcc (absolute target, hyphenated condition)
multiclass SPEX64_BCC_COND<string cond, bits<9> k9val> {
  def BCC_#cond#_32 : SPEX64BCCInst<"bcc-"#cond#"\t$target", (outs),
                                   (ins SPEX64Imm32:$target), k9val, 0> {
  }
  def BCC_#cond#_64 : SPEX64BCCInst<"bcc-"#cond#"\t$target", (outs),
                                   (ins SPEX64Imm64:$target), k9val, 1> {
  }
}
defm BCC_EQ  : SPEX64_BCC_COND<"eq",  0>;
defm BCC_NE  : SPEX64_BCC_COND<"ne",  1>;
defm BCC_LT  : SPEX64_BCC_COND<"lt",  2>;
defm BCC_GE  : SPEX64_BCC_COND<"ge",  3>;
defm BCC_LE  : SPEX64_BCC_COND<"le",  4>;
defm BCC_GT  : SPEX64_BCC_COND<"gt",  5>;
defm BCC_LTU : SPEX64_BCC_COND<"ltu", 6>;
defm BCC_GEU : SPEX64_BCC_COND<"geu", 7>;
defm BCC_LEU : SPEX64_BCC_COND<"leu", 8>;
defm BCC_GTU : SPEX64_BCC_COND<"gtu", 9>;
defm BCC_VS  : SPEX64_BCC_COND<"vs",  10>;
defm BCC_VC  : SPEX64_BCC_COND<"vc",  11>;
defm BCC_MI  : SPEX64_BCC_COND<"mi",  12>;
defm BCC_PL  : SPEX64_BCC_COND<"pl",  13>;
defm BCC_CS  : SPEX64_BCC_COND<"cs",  14>;
defm BCC_CC  : SPEX64_BCC_COND<"cc",  15>;

// jmp / call / sp
def JMP_R : SPEX64Inst<"jmp\t$r6", (outs), (ins GPR:$r6), 0x200, 0, 0b00, 0, 0>;
def JMP32 : SPEX64InstNoR6<"jmp\t$target", (outs), (ins SPEX64Imm32:$target),
                           0x200, 0, 0b00, 1, 0> {
}
def JMP64 : SPEX64InstNoR6<"jmp\t$target", (outs), (ins SPEX64Imm64:$target),
                           0x200, 0, 0b00, 1, 1> {
}

def CALL_R : SPEX64Inst<"call\t$r6", (outs), (ins GPR:$r6), 0x203, 0, 0b00, 0, 0>;
def CALL32 : SPEX64InstNoR6<"call\t$target", (outs), (ins SPEX64Imm32:$target),
                            0x203, 0, 0b00, 1, 0> {
}
def CALL64 : SPEX64InstNoR6<"call\t$target", (outs), (ins SPEX64Imm64:$target),
                            0x203, 0, 0b00, 1, 1> {
}

def SP_RX : SPEX64Inst<"sp\t$r6", (outs), (ins GPR:$r6), 0x206, 0, 0b00, 0, 0>;
def SP32 : SPEX64Inst<"sp\t$r6, $target", (outs),
                      (ins GPR:$r6, SPEX64Imm32:$target),
                      0x206, 0, 0b00, 1, 0> {
}
def SP64 : SPEX64Inst<"sp\t$r6, $target", (outs),
                      (ins GPR:$r6, SPEX64Imm64:$target),
                      0x206, 0, 0b00, 1, 1> {
}

// mov* (rx <-> r6). Lane-local variants are represented as separate mnemonics.
multiclass SPEX64_MOV_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def MOV#def_suf : SPEX64Inst<"mov"#asm_suf#"\trx, $r6", (outs),
                               (ins GPR:$r6), 0x300, 0, sz2, 0, 0>;
  def MOV#def_suf#"_R" : SPEX64Inst<"mov"#asm_suf#"\t$r6, rx", (outs),
                                   (ins GPR:$r6), 0x300, 1, sz2, 0, 0>;
  def MOV#def_suf#"_CX0" : SPEX64InstNoR6<"mov"#asm_suf#"\tcx0", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1" : SPEX64InstNoR6<"mov"#asm_suf#"\tcx1", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS" : SPEX64InstNoR6<"mov"#asm_suf#"\tcas", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let K9 = 3;
  }
  def MOV#def_suf#"_CX0_R" : SPEX64InstNoR6<"mov"#asm_suf#"\tcx0, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1_R" : SPEX64InstNoR6<"mov"#asm_suf#"\tcx1, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS_R" : SPEX64InstNoR6<"mov"#asm_suf#"\tcas, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let K9 = 3;
  }
}
defm MOV : SPEX64_MOV_SZ<"8",  "8",  0b00>;
defm MOV : SPEX64_MOV_SZ<"16", "16", 0b01>;
defm MOV : SPEX64_MOV_SZ<"32", "32", 0b10>;
defm MOV : SPEX64_MOV_SZ<"64", "64", 0b11>;

multiclass SPEX64_MOV_ALIAS_SZ<string def_suf, string asm_suf> {
  def : InstAlias<"mov"#asm_suf#" rx, $r6",
                  (!cast<Instruction>("MOVMOV"#def_suf) GPR:$r6)>;
  def : InstAlias<"mov"#asm_suf#" rx, cx0",
                  (!cast<Instruction>("MOVMOV"#def_suf#"_CX0"))>;
  def : InstAlias<"mov"#asm_suf#" rx, cx1",
                  (!cast<Instruction>("MOVMOV"#def_suf#"_CX1"))>;
  def : InstAlias<"mov"#asm_suf#" rx, cas",
                  (!cast<Instruction>("MOVMOV"#def_suf#"_CAS"))>;
}
defm MOV_ALIAS : SPEX64_MOV_ALIAS_SZ<"8",  "8">;
defm MOV_ALIAS : SPEX64_MOV_ALIAS_SZ<"16", "16">;
defm MOV_ALIAS : SPEX64_MOV_ALIAS_SZ<"32", "32">;
defm MOV_ALIAS : SPEX64_MOV_ALIAS_SZ<"64", "64">;

// li*
multiclass SPEX64_LI_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def LI#def_suf#"_32" : SPEX64InstNoR6<"li"#asm_suf#"\trx, $imm", (outs), (ins SPEX64Imm32:$imm), 0x302, 0, sz2, 1, 0> {
  }
  def LI#def_suf#"_64" : SPEX64InstNoR6<"li"#asm_suf#"\trx, $imm", (outs), (ins SPEX64Imm64:$imm), 0x302, 0, sz2, 1, 1> {
  }
}
defm LI : SPEX64_LI_SZ<"8",  "8",  0b00>;
defm LI : SPEX64_LI_SZ<"16", "16", 0b01>;
defm LI : SPEX64_LI_SZ<"32", "32", 0b10>;
defm LI : SPEX64_LI_SZ<"64", "64", 0b11>;

// Binary ALU (reg + imm32 + imm64)
multiclass SPEX64_ALU_SZ<bits<12> opc, string mn, string def_suf,
                         string asm_suf, bits<2> sz2> {
  def NAME#def_suf#"_R" : SPEX64Inst<mn#asm_suf#"\t$r6", (outs),
                                    (ins GPR:$r6),
                                    opc, 0, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64InstNoR6<mn#asm_suf#"\t$imm", (outs),
                                      (ins SPEX64Imm32:$imm),
                                      opc, 0, sz2, 1, 0> {
  }
  def NAME#def_suf#"_I64" : SPEX64InstNoR6<mn#asm_suf#"\t$imm", (outs),
                                      (ins SPEX64Imm64:$imm),
                                      opc, 0, sz2, 1, 1> {
  }
}
multiclass SPEX64_ALU<bits<12> opc, string mn> {
  defm NAME : SPEX64_ALU_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX64_ALU_SZ<opc, mn, "64", "64", 0b11>;
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
  def NAME#def_suf : SPEX64InstNoR6<mn#asm_suf, (outs), (ins),
                               opc, 0, sz2, 0, 0>;
}
multiclass SPEX64_UNARY<bits<12> opc, string mn> {
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX64_UNARY_SZ<opc, mn, "64", "64", 0b11>;
}
defm NOT : SPEX64_UNARY<0x410, "not">;
defm NEG : SPEX64_UNARY<0x411, "neg">;
defm INC : SPEX64_UNARY<0x412, "inc">;
defm DEC : SPEX64_UNARY<0x413, "dec">;

// Shift / bit (imm32 only)
multiclass SPEX64_SHIFT_SZ<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2> {
  def NAME#def_suf : SPEX64InstNoR6<mn#asm_suf#"\t$imm", (outs),
                                (ins SPEX64Imm32:$imm),
                                opc, 0, sz2, 1, 0> {
  }
}
multiclass SPEX64_SHIFT<bits<12> opc, string mn> {
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX64_SHIFT_SZ<opc, mn, "64", "64", 0b11>;
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
  def NAME#def_suf : SPEX64InstNoR6<mn#asm_suf#"\t[rx]", (outs), (ins),
                               opc, x1, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64InstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEX64Imm32:$imm),
                                      opc, x1, sz2, 1, 0> {
  }
  def NAME#def_suf#"_I64" : SPEX64InstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEX64Imm64:$imm),
                                      opc, x1, sz2, 1, 1> {
  }
}
multiclass SPEX64_LS_R6<bits<12> opc, string mn, string def_suf, string asm_suf,
                        bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf#"\t$r6, [rx]", (outs), (ins GPR:$r6),
                               opc, x1, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64Inst<mn#asm_suf#"\t$r6, [rx + $imm]", (outs),
                                      (ins GPR:$r6, SPEX64Imm32:$imm),
                                      opc, x1, sz2, 1, 0> {
  }
  def NAME#def_suf#"_I64" : SPEX64Inst<mn#asm_suf#"\t$r6, [rx + $imm]", (outs),
                                      (ins GPR:$r6, SPEX64Imm64:$imm),
                                      opc, x1, sz2, 1, 1> {
  }
}
multiclass SPEX64_ST_R6<bits<12> opc, string mn, string def_suf, string asm_suf,
                        bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEX64Inst<mn#asm_suf#"\t[rx], $r6", (outs), (ins GPR:$r6),
                               opc, x1, sz2, 0, 0>;
  def NAME#def_suf#"_I32" : SPEX64Inst<mn#asm_suf#"\t[rx + $imm], $r6", (outs),
                                      (ins GPR:$r6, SPEX64Imm32:$imm),
                                      opc, x1, sz2, 1, 0> {
  }
  def NAME#def_suf#"_I64" : SPEX64Inst<mn#asm_suf#"\t[rx + $imm], $r6", (outs),
                                      (ins GPR:$r6, SPEX64Imm64:$imm),
                                      opc, x1, sz2, 1, 1> {
  }
}
multiclass SPEX64_LS_NO_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX64_LS_NO_R6<opc, mn, "64", "64", 0b11, x1>;
}
multiclass SPEX64_LS_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX64_LS_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX64_LS_R6<opc, mn, "64", "64", 0b11, x1>;
}
multiclass SPEX64_ST_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX64_ST_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX64_ST_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX64_ST_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX64_ST_R6<opc, mn, "64", "64", 0b11, x1>;
}

defm LDZ : SPEX64_LS_NO_R6_SZS<0x700, "ldz", 0>;
defm LDZ_R : SPEX64_LS_R6_SZS<0x700, "ldz", 1>;
defm LDS : SPEX64_LS_NO_R6_SZS<0x701, "lds", 0>;
defm LDS_R : SPEX64_LS_R6_SZS<0x701, "lds", 1>;
defm ST : SPEX64_LS_NO_R6_SZS<0x702, "st", 0>;
defm ST_R : SPEX64_ST_R6_SZS<0x702, "st", 1>;

// Atomic
def CAS64 : SPEX64Inst<"cas64\t$r6", (outs), (ins GPR:$r6), 0x800, 0, 0b11, 0, 0>;

// CSR / MMU / IPI
def CSRR : SPEX64InstNoR6<"csrr\t$imm", (outs), (ins SPEX64Imm32:$imm),
                      0x900, 0, 0b00, 1, 0> {
}
def CSRW : SPEX64InstNoR6<"csrw\t$imm", (outs), (ins SPEX64Imm32:$imm),
                      0x902, 0, 0b00, 1, 0> {
}
def TLBFLUSH : SPEX64InstNoR6<"tlbflush", (outs), (ins), 0x910, 0, 0b00, 0, 0>;
def TLBFLUSHASID : SPEX64Inst<"tlbflushasid\t$r6", (outs), (ins GPR:$r6),
                              0x911, 0, 0b00, 0, 0>;
def ASIDSET : SPEX64Inst<"asidset\t$r6", (outs), (ins GPR:$r6),
                         0x912, 0, 0b00, 0, 0>;
def IPISEND : SPEX64Inst<"ipisend\t$r6", (outs), (ins GPR:$r6),
                         0x920, 0, 0b00, 0, 0>;

// Lane read (K9 encodes LID and FIELD).
def LRD : SPEX64K9InstNoR6<"lrd\t$k9", (outs), (ins SPEX64Imm9:$k9),
                       0xA00, 0, 0b00, 0, 0>;

defm : RemapAllTargetPseudoPointerOperands<GPR>;
