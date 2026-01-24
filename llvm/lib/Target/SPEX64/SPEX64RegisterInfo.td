//===-- SPEX64RegisterInfo.td - SPEX64 register information --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

class SPEX64Acc<string n> : Register<n> {
  let Namespace = "SPEX64";
}

class SPEX64Reg<string n, bits<16> enc> : Register<n> {
  let Namespace = "SPEX64";
  let HWEncoding = enc;
}

def RX : SPEX64Acc<"rx">;

def LR : SPEX64Acc<"lr">;

def R0  : SPEX64Reg<"r0", 0>;
def R1  : SPEX64Reg<"r1", 1>;
def R2  : SPEX64Reg<"r2", 2>;
def R3  : SPEX64Reg<"r3", 3>;
def R4  : SPEX64Reg<"r4", 4>;
def R5  : SPEX64Reg<"r5", 5>;
def R6  : SPEX64Reg<"r6", 6>;
def R7  : SPEX64Reg<"r7", 7>;

def R8  : SPEX64Reg<"r8", 8>;
def R9  : SPEX64Reg<"r9", 9>;
def R10 : SPEX64Reg<"r10", 10>;
def R11 : SPEX64Reg<"r11", 11>;
def R12 : SPEX64Reg<"r12", 12>;
def R13 : SPEX64Reg<"r13", 13>;
def R14 : SPEX64Reg<"r14", 14>;
def R15 : SPEX64Reg<"r15", 15>;

def R16 : SPEX64Reg<"r16", 16>;
def R17 : SPEX64Reg<"r17", 17>;
def R18 : SPEX64Reg<"r18", 18>;
def R19 : SPEX64Reg<"r19", 19>;
def R20 : SPEX64Reg<"r20", 20>;
def R21 : SPEX64Reg<"r21", 21>;
def R22 : SPEX64Reg<"r22", 22>;
def R23 : SPEX64Reg<"r23", 23>;

def R24 : SPEX64Reg<"r24", 24>;
def R25 : SPEX64Reg<"r25", 25>;
def R26 : SPEX64Reg<"r26", 26>;
def R27 : SPEX64Reg<"r27", 27>;
def R28 : SPEX64Reg<"r28", 28>;
def R29 : SPEX64Reg<"r29", 29>;
def R30 : SPEX64Reg<"r30", 30>;
def R31 : SPEX64Reg<"r31", 31>;

def R32 : SPEX64Reg<"r32", 32>;
def R33 : SPEX64Reg<"r33", 33>;
def R34 : SPEX64Reg<"r34", 34>;
def R35 : SPEX64Reg<"r35", 35>;
def R36 : SPEX64Reg<"r36", 36>;
def R37 : SPEX64Reg<"r37", 37>;
def R38 : SPEX64Reg<"r38", 38>;
def R39 : SPEX64Reg<"r39", 39>;

def R40 : SPEX64Reg<"r40", 40>;
def R41 : SPEX64Reg<"r41", 41>;
def R42 : SPEX64Reg<"r42", 42>;
def R43 : SPEX64Reg<"r43", 43>;
def R44 : SPEX64Reg<"r44", 44>;
def R45 : SPEX64Reg<"r45", 45>;
def R46 : SPEX64Reg<"r46", 46>;
def R47 : SPEX64Reg<"r47", 47>;

def R48 : SPEX64Reg<"r48", 48>;
def R49 : SPEX64Reg<"r49", 49>;
def R50 : SPEX64Reg<"r50", 50>;
def R51 : SPEX64Reg<"r51", 51>;
def R52 : SPEX64Reg<"r52", 52>;
def R53 : SPEX64Reg<"r53", 53>;
def R54 : SPEX64Reg<"r54", 54>;
def R55 : SPEX64Reg<"r55", 55>;

def R56 : SPEX64Reg<"r56", 56>;
def R57 : SPEX64Reg<"r57", 57>;
def R58 : SPEX64Reg<"r58", 58>;
def R59 : SPEX64Reg<"r59", 59>;
def R60 : SPEX64Reg<"r60", 60>;
def R61 : SPEX64Reg<"r61", 61>;
def R62 : SPEX64Reg<"r62", 62>;
def R63 : SPEX64Reg<"r63", 63>;

def GPR : RegisterClass<"SPEX64", [i64, i32, i16, i8], 64,
  (add R0, R1, R2, R3, R4, R5, R6, R7,
       R8, R9, R10, R11, R12, R13, R14, R15,
       R16, R17, R18, R19, R20, R21, R22, R23,
       R24, R25, R26, R27, R28, R29, R30, R31,
       R32, R33, R34, R35, R36, R37, R38, R39,
       R40, R41, R42, R43, R44, R45, R46, R47,
       R48, R49, R50, R51, R52, R53, R54, R55,
       R56, R57, R58, R59, R60, R61, R62, R63)> {
  let DecoderMethod = "DecodeGPRRegisterClass";
}

def ACC : RegisterClass<"SPEX64", [i64, i32, i16, i8], 64, (add RX)>;

def CSR_SPEX64 : CalleeSavedRegs<(add R16, R17, R18, R19, R20, R21, R22, R23,
                                      R24, R25, R26, R27, R28, R29, R30, R31)>;
