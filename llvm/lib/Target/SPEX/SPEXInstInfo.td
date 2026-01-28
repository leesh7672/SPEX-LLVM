//===-- SPEXInstInfo.td - SPEX instruction properties --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "SPEXRegisterInfo.td"

// -------------------------------------------------- //
// Instruction Operands and Patterns
// -------------------------------------------------- //

def SDT_SPEXCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                             SDTCisVT<1, i32>]>;
def SDT_SPEXCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                           SDTCisVT<1, i32>]>;
def SDT_SPEXCall         : SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>;

def Call         : SDNode<"SPEXISD::CALL", SDT_SPEXCall,
                          [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                           SDNPVariadic]>;
def CallSeqStart : SDNode<"ISD::CALLSEQ_START", SDT_SPEXCallSeqStart,
                          [SDNPHasChain, SDNPOutGlue]>;
def CallSeqEnd   : SDNode<"ISD::CALLSEQ_END", SDT_SPEXCallSeqEnd,
                          [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def SPEXImm9AsmOperand : AsmOperandClass {
  let Name = "SPEXImm9";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm9";
  let RenderMethod = "addImmOperands";
}
def SPEXImm32AsmOperand : AsmOperandClass {
  let Name = "SPEXImm32";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm32";
  let RenderMethod = "addImmOperands";
}
def SPEXImm64AsmOperand : AsmOperandClass {
  let Name = "SPEXImm64";
  let ParserMethod = "parseImm";
  let PredicateMethod = "isImm64";
  let RenderMethod = "addImmOperands";
}

def SPEXImm9  : Operand<i32> {
  let ParserMatchClass = SPEXImm9AsmOperand;
  let DecoderMethod = "DecodeSPEXImm9";
}
def SPEXImm32 : Operand<i32> {
  let ParserMatchClass = SPEXImm32AsmOperand;
  let DecoderMethod = "DecodeSPEXImm32";
}
def SPEXImm64 : Operand<i64> {
  let ParserMatchClass = SPEXImm64AsmOperand;
  let DecoderMethod = "DecodeSPEXImm64";
}

def brtarget : Operand<OtherVT> {
  let ParserMatchClass = SPEXImm64AsmOperand;
  let EncoderMethod = "getBranchTargetOpValue";
  let DecoderMethod = "DecodeSPEXImmVar";
}

class SPEXInst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                 bits<2> sz2, bits<1> i1, bits<1> i64> : Instruction {
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString      = asmstr;
  let Namespace      = "SPEX";

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

class SPEXInstNoR6<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                     bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEXInst<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  let r6 = 0;
}

class SPEXK9Inst<string asmstr, dag outs, dag ins, bits<12> opc, bits<1> x1,
                   bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEXInst<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  bits<9> k9;
  let K9 = k9;
}

class SPEXK9InstNoR6<string asmstr, dag outs, dag ins, bits<12> opc,
                       bits<1> x1, bits<2> sz2, bits<1> i1, bits<1> i64>
    : SPEXInstNoR6<asmstr, outs, ins, opc, x1, sz2, i1, i64> {
  bits<9> k9;
  let K9 = k9;
}

class SPEXBCCInst<string asmstr, dag outs, dag ins, bits<9> k9val,
                    bits<1> i64>
    : SPEXK9InstNoR6<asmstr, outs, ins, 0x100, 0b0, 0b00, 1, i64> {
  let k9 = k9val;
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
}

def SPEXInstrInfo : InstrInfo;

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
  let Namespace = "SPEX";
  let isPseudo = 1;
}

def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              "#ADJCALLSTACKDOWN $amt1, $amt2", []>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "#ADJCALLSTACKUP $amt1, $amt2", []>;

def : Pat<(CallSeqStart timm:$amt1, timm:$amt2),
          (ADJCALLSTACKDOWN timm:$amt1, timm:$amt2)>;
def : Pat<(CallSeqEnd timm:$amt1, timm:$amt2),
          (ADJCALLSTACKUP timm:$amt1, timm:$amt2)>;

// SYS opcodes (0x000-0x00E)
def NOP     : SPEXInstNoR6<"nop", (outs), (ins), 0x000, 0, 0b00, 0, 0>;
def HLT     : SPEXInstNoR6<"hlt", (outs), (ins), 0x001, 0, 0b00, 0, 0>;
let isReturn = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 1, Uses = [LR] in
def RET     : SPEXInstNoR6<"ret", (outs), (ins), 0x002, 0, 0b00, 0, 0>;
def SYSCALL : SPEXInstNoR6<"syscall", (outs), (ins), 0x003, 0, 0b00, 0, 0>;
def TRAP    : SPEXInstNoR6<"trap", (outs), (ins), 0x004, 0, 0b00, 0, 0>;
def ERET    : SPEXInstNoR6<"eret", (outs), (ins), 0x005, 0, 0b00, 0, 0>;
def FENCE   : SPEXInstNoR6<"fence", (outs), (ins), 0x006, 0, 0b00, 0, 0>;
def PAUSE   : SPEXInstNoR6<"pause", (outs), (ins), 0x007, 0, 0b00, 0, 0>;
def INTEN   : SPEXInstNoR6<"inten", (outs), (ins), 0x008, 0, 0b00, 0, 0>;
def INTDIS  : SPEXInstNoR6<"intdis", (outs), (ins), 0x009, 0, 0b00, 0, 0>;
def WFI     : SPEXInstNoR6<"wfi", (outs), (ins), 0x00A, 0, 0b00, 0, 0>;
def ILL     : SPEXInstNoR6<"ill", (outs), (ins), 0x00B, 0, 0b00, 0, 0>;
let hasSideEffects = 1, isBarrier = 1 in {
def LSTOP   : SPEXInstNoR6<"lstop", (outs), (ins), 0x00C, 0, 0b00, 0, 0>;
def LWAIT   : SPEXInstNoR6<"lwait", (outs), (ins), 0x00D, 0, 0b00, 0, 0>;
def LWAKE   : SPEXInstNoR6<"lwake", (outs), (ins), 0x00E, 0, 0b00, 0, 0>;
}

// bcc (absolute target, hyphenated condition)
def BCC_eq_32 : SPEXBCCInst<"bcc-eq\t$target", (outs),
                              (ins brtarget:$target), 0, 0> {
}
def BCC_eq_64 : SPEXBCCInst<"bcc-eq\t$target", (outs),
                              (ins brtarget:$target), 0, 1> {
}
def BCC_ne_32 : SPEXBCCInst<"bcc-ne\t$target", (outs),
                              (ins brtarget:$target), 1, 0> {
}
def BCC_ne_64 : SPEXBCCInst<"bcc-ne\t$target", (outs),
                              (ins brtarget:$target), 1, 1> {
}
def BCC_lt_32 : SPEXBCCInst<"bcc-lt\t$target", (outs),
                              (ins brtarget:$target), 2, 0> {
}
def BCC_lt_64 : SPEXBCCInst<"bcc-lt\t$target", (outs),
                              (ins brtarget:$target), 2, 1> {
}
def BCC_ge_32 : SPEXBCCInst<"bcc-ge\t$target", (outs),
                              (ins brtarget:$target), 3, 0> {
}
def BCC_ge_64 : SPEXBCCInst<"bcc-ge\t$target", (outs),
                              (ins brtarget:$target), 3, 1> {
}
def BCC_le_32 : SPEXBCCInst<"bcc-le\t$target", (outs),
                              (ins brtarget:$target), 4, 0> {
}
def BCC_le_64 : SPEXBCCInst<"bcc-le\t$target", (outs),
                              (ins brtarget:$target), 4, 1> {
}
def BCC_gt_32 : SPEXBCCInst<"bcc-gt\t$target", (outs),
                              (ins brtarget:$target), 5, 0> {
}
def BCC_gt_64 : SPEXBCCInst<"bcc-gt\t$target", (outs),
                              (ins brtarget:$target), 5, 1> {
}
def BCC_ltu_32 : SPEXBCCInst<"bcc-ltu\t$target", (outs),
                               (ins brtarget:$target), 6, 0> {
}
def BCC_ltu_64 : SPEXBCCInst<"bcc-ltu\t$target", (outs),
                               (ins brtarget:$target), 6, 1> {
}
def BCC_geu_32 : SPEXBCCInst<"bcc-geu\t$target", (outs),
                               (ins brtarget:$target), 7, 0> {
}
def BCC_geu_64 : SPEXBCCInst<"bcc-geu\t$target", (outs),
                               (ins brtarget:$target), 7, 1> {
}
def BCC_leu_32 : SPEXBCCInst<"bcc-leu\t$target", (outs),
                               (ins brtarget:$target), 8, 0> {
}
def BCC_leu_64 : SPEXBCCInst<"bcc-leu\t$target", (outs),
                               (ins brtarget:$target), 8, 1> {
}
def BCC_gtu_32 : SPEXBCCInst<"bcc-gtu\t$target", (outs),
                               (ins brtarget:$target), 9, 0> {
}
def BCC_gtu_64 : SPEXBCCInst<"bcc-gtu\t$target", (outs),
                               (ins brtarget:$target), 9, 1> {
}
def BCC_vs_32 : SPEXBCCInst<"bcc-vs\t$target", (outs),
                              (ins brtarget:$target), 10, 0> {
}
def BCC_vs_64 : SPEXBCCInst<"bcc-vs\t$target", (outs),
                              (ins brtarget:$target), 10, 1> {
}
def BCC_vc_32 : SPEXBCCInst<"bcc-vc\t$target", (outs),
                              (ins brtarget:$target), 11, 0> {
}
def BCC_vc_64 : SPEXBCCInst<"bcc-vc\t$target", (outs),
                              (ins brtarget:$target), 11, 1> {
}
def BCC_mi_32 : SPEXBCCInst<"bcc-mi\t$target", (outs),
                              (ins brtarget:$target), 12, 0> {
}
def BCC_mi_64 : SPEXBCCInst<"bcc-mi\t$target", (outs),
                              (ins brtarget:$target), 12, 1> {
}
def BCC_pl_32 : SPEXBCCInst<"bcc-pl\t$target", (outs),
                              (ins brtarget:$target), 13, 0> {
}
def BCC_pl_64 : SPEXBCCInst<"bcc-pl\t$target", (outs),
                              (ins brtarget:$target), 13, 1> {
}
def BCC_cs_32 : SPEXBCCInst<"bcc-cs\t$target", (outs),
                              (ins brtarget:$target), 14, 0> {
}
def BCC_cs_64 : SPEXBCCInst<"bcc-cs\t$target", (outs),
                              (ins brtarget:$target), 14, 1> {
}
def BCC_cc_32 : SPEXBCCInst<"bcc-cc\t$target", (outs),
                              (ins brtarget:$target), 15, 0> {
}
def BCC_cc_64 : SPEXBCCInst<"bcc-cc\t$target", (outs),
                              (ins brtarget:$target), 15, 1> {
}

// jmp / call / sp
let isBranch = 1, isTerminator = 1, isBarrier = 1, isIndirectBranch = 1 in
def JMP_R : SPEXInst<"jmp\t$r6", (outs), (ins GPR:$r6), 0x200, 0, 0b00, 0, 0>;

let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JMP32 : SPEXInstNoR6<"jmp	$target", (outs), (ins brtarget:$target),
                           0x200, 0, 0b00, 1, 0> {
}
let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JMP64 : SPEXInstNoR6<"jmp	$target", (outs), (ins brtarget:$target),
                           0x200, 0, 0b00, 1, 1> {
}

let isCall = 1, Defs = [LR] in

// Absolute-address call (inline immediate address). This is what plain `call`
// in assembly means.
def CALL : SPEXInstNoR6<"call\t$imm", (outs), (ins SPEXImm64:$imm, variable_ops),
                           0x203, 0, 0b00, 1, 1> {
}

// Register-indirect calls. The suffix (32/64) indicates the register width.
let isCall = 1, Defs = [LR] in
def CALL32 : SPEXInst<"call32\t$r6", (outs), (ins GPR:$r6, variable_ops),
                        0x203, 0, 0b10, 0, 0>;

let isCall = 1, Defs = [LR] in
def CALL64 : SPEXInst<"call64\t$r6", (outs), (ins GPR:$r6, variable_ops),
                        0x203, 0, 0b11, 0, 0>;

let isCall = 1, isCodeGenOnly = 1 in
def PSEUDO_CALL : Pseudo<(outs), (ins brtarget:$target, variable_ops), "", []>;
let isCall = 1, isCodeGenOnly = 1 in
def PSEUDO_CALLR : Pseudo<(outs), (ins GPR:$r6, variable_ops), "", [(Call GPR:$r6)]>;

def SP_RX : SPEXInst<"sp\t$r6", (outs), (ins GPR:$r6), 0x206, 0, 0b00, 0, 0>;
def SP32 : SPEXInst<"sp	$r6, $target", (outs),
                      (ins GPR:$r6, brtarget:$target),
                      0x206, 0, 0b00, 1, 0> {
}
def SP64 : SPEXInst<"sp	$r6, $target", (outs),
                      (ins GPR:$r6, brtarget:$target),
                      0x206, 0, 0b00, 1, 1> {
}

// mov* (rx <-> r6). Lane-local variants are represented as separate mnemonics.
multiclass SPEX_MOV_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def MOV#def_suf : SPEXInst<"mov"#asm_suf#"\trx, $r6", (outs),
                               (ins GPR:$r6), 0x300, 0, sz2, 0, 0> {
    let Defs = [RX];
  }
  def MOV#def_suf#"_R" : SPEXInst<"mov"#asm_suf#"\t$r6, rx", (outs GPR:$r6),
                                   (ins), 0x300, 1, sz2, 0, 0> {
    let Uses = [RX];
  }
  def MOV#def_suf#"_CX0" : SPEXInstNoR6<"mov"#asm_suf#"\tcx0", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let Defs = [RX];
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1" : SPEXInstNoR6<"mov"#asm_suf#"\tcx1", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let Defs = [RX];
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS" : SPEXInstNoR6<"mov"#asm_suf#"\tcas", (outs), (ins),
                                    0x300, 0, sz2, 0, 0> {
    let Defs = [RX];
    let K9 = 3;
  }
  def MOV#def_suf#"_CX0_R" : SPEXInstNoR6<"mov"#asm_suf#"\tcx0, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let Uses = [RX];
    let K9 = 1;
  }
  def MOV#def_suf#"_CX1_R" : SPEXInstNoR6<"mov"#asm_suf#"\tcx1, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let Uses = [RX];
    let K9 = 2;
  }
  def MOV#def_suf#"_CAS_R" : SPEXInstNoR6<"mov"#asm_suf#"\tcas, rx", (outs), (ins),
                                      0x300, 1, sz2, 0, 0> {
    let Uses = [RX];
    let K9 = 3;
  }
}
defm MOV: SPEX_MOV_SZ<"8",  "8",  0b00>;
defm MOV: SPEX_MOV_SZ<"16", "16", 0b01>;
defm MOV: SPEX_MOV_SZ<"32", "32", 0b10>;
defm MOV: SPEX_MOV_SZ<"64", "64", 0b11>;

// li*
multiclass SPEX_LI_SZ<string def_suf, string asm_suf, bits<2> sz2> {
  def LI#def_suf#"_32" : SPEXInstNoR6<"li"#asm_suf#"\trx, $imm", (outs), (ins SPEXImm32:$imm), 0x302, 0, sz2, 1, 0> {
    let Defs = [RX];
  }
  def LI#def_suf#"_64" : SPEXInstNoR6<"li"#asm_suf#"\trx, $imm", (outs), (ins SPEXImm64:$imm), 0x302, 0, sz2, 1, 1> {
    let Defs = [RX];
  }
}
defm LI : SPEX_LI_SZ<"8",  "8",  0b00>;
defm LI : SPEX_LI_SZ<"16", "16", 0b01>;
defm LI : SPEX_LI_SZ<"32", "32", 0b10>;
defm LI : SPEX_LI_SZ<"64", "64", 0b11>;

// Binary ALU (reg + imm32 + imm64)
multiclass SPEX_ALU_SZ<bits<12> opc, string mn, string def_suf,
                         string asm_suf, bits<2> sz2> {
  def NAME#def_suf#"_R" : SPEXInst<mn#asm_suf#"\trx, $r6", (outs),
                                    (ins GPR:$r6),
                                    opc, 0, sz2, 0, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInstNoR6<mn#asm_suf#"\trx, $imm", (outs),
                                      (ins SPEXImm32:$imm),
                                      opc, 0, sz2, 1, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInstNoR6<mn#asm_suf#"\trx, $imm", (outs),
                                      (ins SPEXImm64:$imm),
                                      opc, 0, sz2, 1, 1> {
    let Uses = [RX];
    let Defs = [RX];
  }
}
multiclass SPEX_ALU<bits<12> opc, string mn> {
  defm NAME : SPEX_ALU_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX_ALU_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX_ALU_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX_ALU_SZ<opc, mn, "64", "64", 0b11>;
}
multiclass SPEX_ALUFLAGS_SZ<bits<12> opc, string mn, string def_suf,
                              string asm_suf, bits<2> sz2> {
  def NAME#def_suf#"_R" : SPEXInst<mn#asm_suf#"\trx, $r6", (outs),
                                    (ins GPR:$r6),
                                    opc, 0, sz2, 0, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInstNoR6<mn#asm_suf#"\trx, $imm", (outs),
                                     (ins SPEXImm32:$imm),
                                     opc, 0, sz2, 1, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInstNoR6<mn#asm_suf#"\trx, $imm", (outs),
                                     (ins SPEXImm64:$imm),
                                     opc, 0, sz2, 1, 1> {
    let Uses = [RX];
  }
}
multiclass SPEX_ALUFLAGS<bits<12> opc, string mn> {
  defm NAME : SPEX_ALUFLAGS_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX_ALUFLAGS_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX_ALUFLAGS_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX_ALUFLAGS_SZ<opc, mn, "64", "64", 0b11>;
}
defm ADD  : SPEX_ALU<0x400, "add">;
defm SUB  : SPEX_ALU<0x401, "sub">;
defm AND  : SPEX_ALU<0x402, "and">;
defm OR   : SPEX_ALU<0x403, "or">;
defm XOR  : SPEX_ALU<0x404, "xor">;
defm CMP  : SPEX_ALUFLAGS<0x405, "cmp">;
defm TEST : SPEX_ALUFLAGS<0x406, "test">;

// Unary ALU
multiclass SPEX_UNARY_SZ<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2> {
  def NAME#def_suf : SPEXInstNoR6<mn#asm_suf, (outs), (ins),
                               opc, 0, sz2, 0, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
}
multiclass SPEX_UNARY<bits<12> opc, string mn> {
  defm NAME : SPEX_UNARY_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX_UNARY_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX_UNARY_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX_UNARY_SZ<opc, mn, "64", "64", 0b11>;
}
defm NOT : SPEX_UNARY<0x410, "not">;
defm NEG : SPEX_UNARY<0x411, "neg">;
defm INC : SPEX_UNARY<0x412, "inc">;
defm DEC : SPEX_UNARY<0x413, "dec">;

// Shift / bit (imm32 only)
multiclass SPEX_SHIFT_SZ<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2> {
  def NAME#def_suf : SPEXInstNoR6<mn#asm_suf#"\t$imm", (outs),
                                (ins SPEXImm32:$imm),
                                opc, 0, sz2, 1, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
}
multiclass SPEX_SHIFT<bits<12> opc, string mn> {
  defm NAME : SPEX_SHIFT_SZ<opc, mn, "8",  "8",  0b00>;
  defm NAME : SPEX_SHIFT_SZ<opc, mn, "16", "16", 0b01>;
  defm NAME : SPEX_SHIFT_SZ<opc, mn, "32", "32", 0b10>;
  defm NAME : SPEX_SHIFT_SZ<opc, mn, "64", "64", 0b11>;
}
defm SHL  : SPEX_SHIFT<0x600, "shl">;
defm SHR  : SPEX_SHIFT<0x601, "shr">;
defm SAR  : SPEX_SHIFT<0x602, "sar">;
defm BTST : SPEX_SHIFT<0x610, "btst">;
defm BSET : SPEX_SHIFT<0x611, "bset">;
defm BCLR : SPEX_SHIFT<0x612, "bclr">;
defm BINV : SPEX_SHIFT<0x613, "binv">;

// Load / store (base rx, optional offset)
multiclass SPEX_LS_NO_R6<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEXInstNoR6<mn#asm_suf#"\t[rx]", (outs), (ins),
                               opc, x1, sz2, 0, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEXImm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEXImm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Uses = [RX];
  }
}
multiclass SPEX_LS_R6<bits<12> opc, string mn, string def_suf, string asm_suf,
                        bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEXInst<mn#asm_suf#"\t$r6, [rx]", (outs GPR:$r6),
                               (ins), opc, x1, sz2, 0, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInst<mn#asm_suf#"\t$r6, [rx + $imm]",
                                      (outs GPR:$r6),
                                      (ins SPEXImm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInst<mn#asm_suf#"\t$r6, [rx + $imm]",
                                      (outs GPR:$r6),
                                      (ins SPEXImm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Uses = [RX];
  }
}
multiclass SPEX_ST_R6<bits<12> opc, string mn, string def_suf, string asm_suf,
                        bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEXInst<mn#asm_suf#"\t[rx], $r6", (outs), (ins GPR:$r6),
                               opc, x1, sz2, 0, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInst<mn#asm_suf#"\t[rx + $imm], $r6", (outs),
                                      (ins GPR:$r6, SPEXImm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Uses = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInst<mn#asm_suf#"\t[rx + $imm], $r6", (outs),
                                      (ins GPR:$r6, SPEXImm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Uses = [RX];
  }
}
multiclass SPEX_LD_NO_R6<bits<12> opc, string mn, string def_suf,
                           string asm_suf, bits<2> sz2, bits<1> x1> {
  def NAME#def_suf : SPEXInstNoR6<mn#asm_suf#"\t[rx]", (outs), (ins),
                               opc, x1, sz2, 0, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
  def NAME#def_suf#"_I32" : SPEXInstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEXImm32:$imm),
                                      opc, x1, sz2, 1, 0> {
    let Uses = [RX];
    let Defs = [RX];
  }
  def NAME#def_suf#"_I64" : SPEXInstNoR6<mn#asm_suf#"\t[rx + $imm]", (outs),
                                      (ins SPEXImm64:$imm),
                                      opc, x1, sz2, 1, 1> {
    let Uses = [RX];
    let Defs = [RX];
  }
}
multiclass SPEX_LS_NO_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX_LS_NO_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX_LS_NO_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX_LS_NO_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX_LS_NO_R6<opc, mn, "64", "64", 0b11, x1>;
}
multiclass SPEX_LD_NO_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX_LD_NO_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX_LD_NO_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX_LD_NO_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX_LD_NO_R6<opc, mn, "64", "64", 0b11, x1>;
}
multiclass SPEX_LS_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX_LS_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX_LS_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX_LS_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX_LS_R6<opc, mn, "64", "64", 0b11, x1>;
}
multiclass SPEX_ST_R6_SZS<bits<12> opc, string mn, bits<1> x1> {
  defm NAME : SPEX_ST_R6<opc, mn, "8",  "8",  0b00, x1>;
  defm NAME : SPEX_ST_R6<opc, mn, "16", "16", 0b01, x1>;
  defm NAME : SPEX_ST_R6<opc, mn, "32", "32", 0b10, x1>;
  defm NAME : SPEX_ST_R6<opc, mn, "64", "64", 0b11, x1>;
}

defm LDZ : SPEX_LD_NO_R6_SZS<0x700, "ldz", 0>;
defm LDZ_R : SPEX_LS_R6_SZS<0x700, "ldz", 1>;
defm LDS : SPEX_LD_NO_R6_SZS<0x701, "lds", 0>;
defm LDS_R : SPEX_LS_R6_SZS<0x701, "lds", 1>;
defm ST : SPEX_LS_NO_R6_SZS<0x702, "st", 0>;
defm ST_R : SPEX_ST_R6_SZS<0x702, "st", 1>;

// Atomic
def CAS64 : SPEXInst<"cas64\t$r6", (outs), (ins GPR:$r6), 0x800, 0, 0b11, 0, 0>;

// CSR / MMU / IPI
def CSRR : SPEXInstNoR6<"csrr\t$imm", (outs), (ins SPEXImm32:$imm),
                      0x900, 0, 0b00, 1, 0> {
  let Defs = [RX];
  let hasSideEffects = 1;
}
def CSRW : SPEXInstNoR6<"csrw\t$imm", (outs), (ins SPEXImm32:$imm),
                      0x902, 0, 0b00, 1, 0> {
  let Uses = [RX];
  let hasSideEffects = 1;
}
def TLBFLUSH : SPEXInstNoR6<"tlbflush", (outs), (ins), 0x910, 0, 0b00, 0, 0>;
def TLBFLUSHASID : SPEXInst<"tlbflushasid\t$r6", (outs), (ins GPR:$r6),
                              0x911, 0, 0b00, 0, 0>;
def ASIDSET : SPEXInst<"asidset\t$r6", (outs), (ins GPR:$r6),
                         0x912, 0, 0b00, 0, 0>;
def IPISEND : SPEXInst<"ipisend\t$r6", (outs), (ins GPR:$r6),
                         0x920, 0, 0b00, 0, 0>;

// Lane read (K9 encodes LID and FIELD).
def LRD : SPEXK9InstNoR6<"lrd\t$k9", (outs), (ins SPEXImm9:$k9),
                       0xA00, 0, 0b00, 0, 0>;

// SelectionDAG patterns and pseudos.
def SPEXImm32Pat : PatLeaf<(i32 imm)>;
def SPEXImm64Pat : PatLeaf<(i64 imm)>;
def SPEXAddr : ComplexPattern<i64, 2, "SelectAddr", [add, sub, frameindex]>;
def SPEXAddrRR : ComplexPattern<i64, 2, "SelectAddrRR", [add, sub]>;

class SPEXPseudoInst<dag outs, dag ins> : Instruction {
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = "";
  let Namespace = "SPEX";
  let isPseudo = 1;
  let isCodeGenOnly = 1;
  let hasNoSchedulingInfo = 1;
}

let hasSideEffects = 1, isBranch = 1, isTerminator = 1, isBarrier = 1 in {
  def PSEUDO_BR : SPEXPseudoInst<(outs), (ins brtarget:$dst)>;
  def PSEUDO_BR_CC32 : SPEXPseudoInst<(outs),
                                       (ins GPR:$lhs, GPR:$rhs, brtarget:$dst,
                                            i32imm:$cc)>;
  def PSEUDO_BR_CC64 : SPEXPseudoInst<(outs),
                                       (ins GPR:$lhs, GPR:$rhs, brtarget:$dst,
                                            i32imm:$cc)>;
}

def PSEUDO_LI32 : SPEXPseudoInst<(outs GPR:$dst), (ins SPEXImm32:$imm)>;
def PSEUDO_LI64 : SPEXPseudoInst<(outs GPR:$dst), (ins SPEXImm64:$imm)>;
def PSEUDO_LI16 : SPEXPseudoInst<(outs GPR:$dst), (ins SPEXImm32:$imm)>;
def PSEUDO_LI8 : SPEXPseudoInst<(outs GPR:$dst), (ins SPEXImm32:$imm)>;

def PSEUDO_ADD32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_ADD64rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_ADD32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_ADD64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i64imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_SUB32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SUB64rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SUB32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SUB64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i64imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_AND32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_AND64rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_AND32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_AND64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i64imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_OR32rr : SPEXPseudoInst<(outs GPR:$dst),
                                     (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_OR64rr : SPEXPseudoInst<(outs GPR:$dst),
                                     (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_OR32ri : SPEXPseudoInst<(outs GPR:$dst),
                                     (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_OR64ri : SPEXPseudoInst<(outs GPR:$dst),
                                     (ins GPR:$src, i64imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_XOR32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_XOR64rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, GPR:$rhs)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_XOR32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_XOR64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i64imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_SHL32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SHL64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SRL32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SRL64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SRA32ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}
def PSEUDO_SRA64ri : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$src, i32imm:$imm)> {
  let Constraints = "$dst = $src";
}

def PSEUDO_LDZ32 : SPEXPseudoInst<(outs GPR:$dst),
                                    (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDZ64 : SPEXPseudoInst<(outs GPR:$dst),
                                    (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDZ16 : SPEXPseudoInst<(outs GPR:$dst),
                                    (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDZ8 : SPEXPseudoInst<(outs GPR:$dst),
                                   (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDS32 : SPEXPseudoInst<(outs GPR:$dst),
                                    (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDS16 : SPEXPseudoInst<(outs GPR:$dst),
                                    (ins GPR:$base, i32imm:$off)>;
def PSEUDO_LDS8 : SPEXPseudoInst<(outs GPR:$dst),
                                   (ins GPR:$base, i32imm:$off)>;
def PSEUDO_ST32 : SPEXPseudoInst<(outs),
                                   (ins GPR:$val, GPR:$base, i32imm:$off)>;
def PSEUDO_ST64 : SPEXPseudoInst<(outs),
                                   (ins GPR:$val, GPR:$base, i32imm:$off)>;
def PSEUDO_ST16 : SPEXPseudoInst<(outs),
                                   (ins GPR:$val, GPR:$base, i32imm:$off)>;
def PSEUDO_ST8 : SPEXPseudoInst<(outs),
                                  (ins GPR:$val, GPR:$base, i32imm:$off)>;

def PSEUDO_LDZ32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;
def PSEUDO_LDZ64rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;
def PSEUDO_LDZ16rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;
def PSEUDO_LDZ8rr  : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;

def PSEUDO_LDS32rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;
def PSEUDO_LDS16rr : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;
def PSEUDO_LDS8rr  : SPEXPseudoInst<(outs GPR:$dst),
                                      (ins GPR:$base, GPR:$idx)>;

def PSEUDO_ST32rr  : SPEXPseudoInst<(outs),
                                      (ins GPR:$val, GPR:$base, GPR:$idx)>;
def PSEUDO_ST64rr  : SPEXPseudoInst<(outs),
                                      (ins GPR:$val, GPR:$base, GPR:$idx)>;
def PSEUDO_ST16rr  : SPEXPseudoInst<(outs),
                                      (ins GPR:$val, GPR:$base, GPR:$idx)>;
def PSEUDO_ST8rr   : SPEXPseudoInst<(outs),
                                      (ins GPR:$val, GPR:$base, GPR:$idx)>;


def : Pat<(i32 imm:$imm), (PSEUDO_LI32 imm:$imm)>;
def : Pat<(i64 imm:$imm), (PSEUDO_LI64 imm:$imm)>;
def : Pat<(i32 imm:$imm), (PSEUDO_LI16 imm:$imm)>;
def : Pat<(i32 imm:$imm), (PSEUDO_LI8 imm:$imm)>;
def : Pat<(i64 tglobaladdr:$imm), (PSEUDO_LI64 tglobaladdr:$imm)>;
def : Pat<(i64 texternalsym:$imm), (PSEUDO_LI64 texternalsym:$imm)>;

def : Pat<(add i32:$lhs, i32:$rhs),
          (PSEUDO_ADD32rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(add i64:$lhs, i64:$rhs),
          (PSEUDO_ADD64rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(add i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_ADD32ri GPR:$lhs, imm:$imm)>;
def : Pat<(add i64:$lhs, SPEXImm64Pat:$imm),
          (PSEUDO_ADD64ri GPR:$lhs, imm:$imm)>;

def : Pat<(sub i32:$lhs, i32:$rhs),
          (PSEUDO_SUB32rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(sub i64:$lhs, i64:$rhs),
          (PSEUDO_SUB64rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(sub i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SUB32ri GPR:$lhs, imm:$imm)>;
def : Pat<(sub i64:$lhs, SPEXImm64Pat:$imm),
          (PSEUDO_SUB64ri GPR:$lhs, imm:$imm)>;

def : Pat<(and i32:$lhs, i32:$rhs),
          (PSEUDO_AND32rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(and i64:$lhs, i64:$rhs),
          (PSEUDO_AND64rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(and i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_AND32ri GPR:$lhs, imm:$imm)>;
def : Pat<(and i64:$lhs, SPEXImm64Pat:$imm),
          (PSEUDO_AND64ri GPR:$lhs, imm:$imm)>;

def : Pat<(or i32:$lhs, i32:$rhs),
          (PSEUDO_OR32rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(or i64:$lhs, i64:$rhs),
          (PSEUDO_OR64rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(or i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_OR32ri GPR:$lhs, imm:$imm)>;
def : Pat<(or i64:$lhs, SPEXImm64Pat:$imm),
          (PSEUDO_OR64ri GPR:$lhs, imm:$imm)>;

def : Pat<(xor i32:$lhs, i32:$rhs),
          (PSEUDO_XOR32rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(xor i64:$lhs, i64:$rhs),
          (PSEUDO_XOR64rr GPR:$lhs, GPR:$rhs)>;
def : Pat<(xor i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_XOR32ri GPR:$lhs, imm:$imm)>;
def : Pat<(xor i64:$lhs, SPEXImm64Pat:$imm),
          (PSEUDO_XOR64ri GPR:$lhs, imm:$imm)>;

def : Pat<(shl i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SHL32ri GPR:$lhs, imm:$imm)>;
def : Pat<(shl i64:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SHL64ri GPR:$lhs, imm:$imm)>;
def : Pat<(srl i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SRL32ri GPR:$lhs, imm:$imm)>;
def : Pat<(srl i64:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SRL64ri GPR:$lhs, imm:$imm)>;
def : Pat<(sra i32:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SRA32ri GPR:$lhs, imm:$imm)>;
def : Pat<(sra i64:$lhs, SPEXImm32Pat:$imm),
          (PSEUDO_SRA64ri GPR:$lhs, imm:$imm)>;

def : Pat<(i64 (anyext i8:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;
def : Pat<(i64 (anyext i16:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;
def : Pat<(i64 (anyext i32:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;
def : Pat<(i8 (trunc i64:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;
def : Pat<(i16 (trunc i64:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;
def : Pat<(i32 (trunc i64:$src)),
          (PSEUDO_OR64ri GPR:$src, (i64 0))>;


def : Pat<(i32 (load (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ32rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (load (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ64rr GPR:$base, GPR:$idx)>;
def : Pat<(i16 (load (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;
def : Pat<(i8 (load (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;

def : Pat<(i32 (zext (i8 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (zext (i8 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (zext (i16 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (zext (i16 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (sext (i8 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDS8rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (sext (i8 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDS8rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (sext (i16 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDS16rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (sext (i16 (load (SPEXAddrRR i64:$base, i64:$idx))))),
          (PSEUDO_LDS16rr GPR:$base, GPR:$idx)>;

def : Pat<(i64 (zextloadi8 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (zextloadi16 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (zextloadi32 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ32rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (zextloadi8 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (zextloadi16 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;

def : Pat<(i64 (extloadi8 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (extloadi16 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (extloadi32 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ32rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (extloadi8 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ8rr GPR:$base, GPR:$idx)>;
def : Pat<(i32 (extloadi16 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDZ16rr GPR:$base, GPR:$idx)>;

def : Pat<(i64 (sextloadi8 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDS8rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (sextloadi16 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDS16rr GPR:$base, GPR:$idx)>;
def : Pat<(i64 (sextloadi32 (SPEXAddrRR i64:$base, i64:$idx))),
          (PSEUDO_LDS32rr GPR:$base, GPR:$idx)>;

def : Pat<(store i32:$val, (SPEXAddrRR i64:$base, i64:$idx)),
          (PSEUDO_ST32rr GPR:$val, GPR:$base, GPR:$idx)>;
def : Pat<(store i64:$val, (SPEXAddrRR i64:$base, i64:$idx)),
          (PSEUDO_ST64rr GPR:$val, GPR:$base, GPR:$idx)>;
def : Pat<(store i16:$val, (SPEXAddrRR i64:$base, i64:$idx)),
          (PSEUDO_ST16rr GPR:$val, GPR:$base, GPR:$idx)>;
def : Pat<(store i8:$val, (SPEXAddrRR i64:$base, i64:$idx)),
          (PSEUDO_ST8rr GPR:$val, GPR:$base, GPR:$idx)>;

def : Pat<(i32 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ32 GPR:$base, imm:$off)>;
def : Pat<(i64 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ64 GPR:$base, imm:$off)>;
def : Pat<(i16 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i8 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i32 (zext (i8 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i64 (zext (i8 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i32 (zext (i16 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i64 (zext (i16 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i32 (sext (i8 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDS8 GPR:$base, imm:$off)>;
def : Pat<(i64 (sext (i8 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDS8 GPR:$base, imm:$off)>;
def : Pat<(i32 (sext (i16 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDS16 GPR:$base, imm:$off)>;
def : Pat<(i64 (sext (i16 (load (SPEXAddr i64:$base, SPEXImm32Pat:$off))))),
          (PSEUDO_LDS16 GPR:$base, imm:$off)>;
def : Pat<(i64 (zextloadi8 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i64 (zextloadi16 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i64 (zextloadi32 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ32 GPR:$base, imm:$off)>;
def : Pat<(i32 (zextloadi8 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i32 (zextloadi16 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i64 (extloadi8 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i64 (extloadi16 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i64 (extloadi32 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ32 GPR:$base, imm:$off)>;
def : Pat<(i32 (extloadi8 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ8 GPR:$base, imm:$off)>;
def : Pat<(i32 (extloadi16 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDZ16 GPR:$base, imm:$off)>;
def : Pat<(i64 (sextloadi8 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDS8 GPR:$base, imm:$off)>;
def : Pat<(i64 (sextloadi16 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDS16 GPR:$base, imm:$off)>;
def : Pat<(i64 (sextloadi32 (SPEXAddr i64:$base, SPEXImm32Pat:$off))),
          (PSEUDO_LDS32 GPR:$base, imm:$off)>;
def : Pat<(store i32:$val, (SPEXAddr i64:$base, SPEXImm32Pat:$off)),
          (PSEUDO_ST32 GPR:$val, GPR:$base, imm:$off)>;
def : Pat<(store i64:$val, (SPEXAddr i64:$base, SPEXImm32Pat:$off)),
          (PSEUDO_ST64 GPR:$val, GPR:$base, imm:$off)>;
def : Pat<(store i16:$val, (SPEXAddr i64:$base, SPEXImm32Pat:$off)),
          (PSEUDO_ST16 GPR:$val, GPR:$base, imm:$off)>;
def : Pat<(store i8:$val, (SPEXAddr i64:$base, SPEXImm32Pat:$off)),
          (PSEUDO_ST8 GPR:$val, GPR:$base, imm:$off)>;

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def : Pat<(br bb:$dst), (PSEUDO_BR brtarget:$dst)>;
