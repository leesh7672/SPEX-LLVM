//===-- SPEXInst.td - SPEX instructions --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//===-- SPEXInst.td -----------------------------------*- tablegen -*-===//

class SPEX_W0<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_NoR6_NoK9<a, o, i, opc, x1, sz2, 0, 0>;

class SPEX_W0_I32<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_NoR6_NoK9<a, o, i, opc, x1, sz2, 1, 0>;

class SPEX_W0_I64<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_NoR6_NoK9<a, o, i, opc, x1, sz2, 1, 1>;

class SPEX_W0_R6<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_R6_NoK9<a, o, i, opc, x1, sz2, 0, 0>;

class SPEX_W0_R6_I32<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_R6_NoK9<a, o, i, opc, x1, sz2, 1, 0>;

class SPEX_W0_R6_I64<string a, dag o, dag i, bits<12> opc, bits<1> x1, bits<2> sz2>
  : SPEXInst_R6_NoK9<a, o, i, opc, x1, sz2, 1, 1>;
