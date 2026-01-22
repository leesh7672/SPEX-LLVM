//===-- SPEX64CallingConv.td - Calling Convention ----------------*- tablegen -*-===//

include "llvm/Target/TargetCallingConv.td"

def CC_SPEX64 : CallingConv<[
  CCIfType<[i64], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[i16], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[i8 ], CCAssignToReg<[R0, R1, R2, R3]>>,

  CCAssignToStack<8, 8>
]>;

def RetCC_SPEX64 : CallingConv<[
  CCIfType<[i64], CCAssignToReg<[R0]>>,
  CCIfType<[i32], CCAssignToReg<[R0]>>,
  CCIfType<[i16], CCAssignToReg<[R0]>>,
  CCIfType<[i8 ], CCAssignToReg<[R0]>>
]>;
