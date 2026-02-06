//===-- SPEXRegisterInfo.td - SPEX register information --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

class SPEXAcc<string n> : Register<n> {
  let Namespace = "SPEX";
}

class SPEXReg<string n, bits<16> enc> : Register<n> {
  let Namespace = "SPEX";
  let HWEncoding = enc;
}

def RX : SPEXAcc<"rx">;

def R0  : SPEXReg<"r0", 0>;
def R1  : SPEXReg<"r1", 1>;
def R2  : SPEXReg<"r2", 2>;
def R3  : SPEXReg<"r3", 3>;
def R4  : SPEXReg<"r4", 4>;
def R5  : SPEXReg<"r5", 5>;
def R6  : SPEXReg<"r6", 6>;
def R7  : SPEXReg<"r7", 7>;

def R8  : SPEXReg<"r8", 8>;
def R9  : SPEXReg<"r9", 9>;
def R10 : SPEXReg<"r10", 10>;
def R11 : SPEXReg<"r11", 11>;
def R12 : SPEXReg<"r12", 12>;
def R13 : SPEXReg<"r13", 13>;
def R14 : SPEXReg<"r14", 14>;
def R15 : SPEXReg<"r15", 15>;

def R16 : SPEXReg<"r16", 16>;
def R17 : SPEXReg<"r17", 17>;
def R18 : SPEXReg<"r18", 18>;
def R19 : SPEXReg<"r19", 19>;
def R20 : SPEXReg<"r20", 20>;
def R21 : SPEXReg<"r21", 21>;
def R22 : SPEXReg<"r22", 22>;
def R23 : SPEXReg<"r23", 23>;

def R24 : SPEXReg<"r24", 24>;
def R25 : SPEXReg<"r25", 25>;
def R26 : SPEXReg<"r26", 26>;
def R27 : SPEXReg<"r27", 27>;
def R28 : SPEXReg<"r28", 28>;
def R29 : SPEXReg<"r29", 29>;
def R30 : SPEXReg<"r30", 30>;
def R31 : SPEXReg<"r31", 31>;

def R32 : SPEXReg<"r32", 32>;
def R33 : SPEXReg<"r33", 33>;
def R34 : SPEXReg<"r34", 34>;
def R35 : SPEXReg<"r35", 35>;
def R36 : SPEXReg<"r36", 36>;
def R37 : SPEXReg<"r37", 37>;
def R38 : SPEXReg<"r38", 38>;
def R39 : SPEXReg<"r39", 39>;

def R40 : SPEXReg<"r40", 40>;
def R41 : SPEXReg<"r41", 41>;
def R42 : SPEXReg<"r42", 42>;
def R43 : SPEXReg<"r43", 43>;
def R44 : SPEXReg<"r44", 44>;
def R45 : SPEXReg<"r45", 45>;
def R46 : SPEXReg<"r46", 46>;
def R47 : SPEXReg<"r47", 47>;

def R48 : SPEXReg<"r48", 48>;
def R49 : SPEXReg<"r49", 49>;
def R50 : SPEXReg<"r50", 50>;
def R51 : SPEXReg<"r51", 51>;
def R52 : SPEXReg<"r52", 52>;
def R53 : SPEXReg<"r53", 53>;
def R54 : SPEXReg<"r54", 54>;
def R55 : SPEXReg<"r55", 55>;

def R56 : SPEXReg<"r56", 56>;
def R57 : SPEXReg<"r57", 57>;
def R58 : SPEXReg<"r58", 58>;
def R59 : SPEXReg<"r59", 59>;
def R60 : SPEXReg<"r60", 60>;
def R61 : SPEXReg<"r61", 61>;
def R62 : SPEXReg<"r62", 62>;
def R63 : SPEXReg<"r63", 63>;

def GPR : RegisterClass<"SPEX", [i64, i32, i16, i8], 64,
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

def ACC : RegisterClass<"SPEX", [i64, i32, i16, i8], 64, (add RX)>;

// In the current SPEX RTL model, the GPR file is core-wide and CALL/RET does
// not preserve it. Treat all GPRs as caller-saved so LLVM will spill live
// values across calls as needed.
def CSR_SPEX : CalleeSavedRegs<(add)>;
