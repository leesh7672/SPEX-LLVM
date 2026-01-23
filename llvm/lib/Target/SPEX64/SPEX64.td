//===-- SPEX64.td - SPEX64 target description --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "SPEX64InstInfo.td"
include "SPEX64CallingConv.td"

def SPEX64 : Target {
  let InstructionSet = SPEX64InstrInfo;
}
