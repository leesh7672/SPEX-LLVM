//===-- SPEXInstBase.td - SPEX instruction base --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===-- SPEXInstBase.td ----------------------------------------*- tablegen -*-===//
//
// Core instruction base classes for SPEX W0 encoding.
//
// Design goals:
//  - W0 bit layout is defined exactly once.
//  - Semantic constraints are enforced by selecting the right base class.
//  - Higher-level format names (SPEX_W0*, etc.) are thin wrappers on top.
//
// W0 layout (32-bit):
//   [31:20] OPC12
//   [19]    X1
//   [18:17] SZ2
//   [16]    I1
//   [15]    I64
//   [14:9]  R6
//   [8:0]   K9
//
//===----------------------------------------------------------------------===//

class SPEXInstBase<
    string asmstr, dag outs, dag ins,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64
  > : Instruction {
  let AsmString  = asmstr;
  let OutOperandList = outs;
  let InOperandList  = ins;

  field bits<32> Inst;

  let Inst{31-20} = OPC;
  let Inst{19}    = X1;
  let Inst{18-17} = SZ2;
  let Inst{16}    = I1;
  let Inst{15}    = I64;

  // These are “semantic fields”. Derived classes decide whether they are used.
  bits<6> R6;
  bits<9> K9;
  let Inst{14-9} = R6;
  let Inst{8-0}  = K9;

  // Default properties (tune as needed per family)
  let isCodeGenOnly = 0;
  let isPseudo = 0;
}

// --------------------------------------------------------------------------
// R6/K9 usage variants
// --------------------------------------------------------------------------

// No R6 field usage: force R6 to zero.
class SPEXInst_NoR6<
    string a, dag o, dag i,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64
  > : SPEXInstBase<a, o, i, OPC, X1, SZ2, I1, I64> {
  let R6 = 0;
}

// R6 field is meaningful: subclasses must bind R6 from an operand.
class SPEXInst_R6<
    string a, dag o, dag i,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64
  > : SPEXInstBase<a, o, i, OPC, X1, SZ2, I1, I64> {
  // Do not force R6 here.
}

// No K9 usage: force K9 to zero.
class SPEXInst_NoK9 : Instruction {
  // Intentionally empty: used via multiple inheritance is NOT recommended in td.
  // Keep K9 forcing in concrete classes below to avoid confusion.
}

// K9 usage variants:
//  - “NoK9” forces K9=0
//  - “K9Imm” allows passing K9 as an argument (or binding from operand)
class SPEXInst_NoR6_NoK9<
    string a, dag o, dag i,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64
  > : SPEXInst_NoR6<a, o, i, OPC, X1, SZ2, I1, I64> {
  let K9 = 0;
}

class SPEXInst_R6_NoK9<
    string a, dag o, dag i,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64
  > : SPEXInst_R6<a, o, i, OPC, X1, SZ2, I1, I64> {
  let K9 = 0;
}

// K9 is meaningful and provided as a template argument (common for bcc-like ops).
class SPEXInst_NoR6_K9<
    string a, dag o, dag i,
    bits<12> OPC, bits<1> X1, bits<2> SZ2,
    bits<1> I1, bits<1> I64,
    bits<9> K9Val
  > : SPEXInst_NoR6<a, o, i, OPC, X1, SZ2, I1, I64> {
  let K9 = K9Val;
}
