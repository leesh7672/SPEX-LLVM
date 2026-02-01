//===-- SPEXISelLowering.h - SPEX lowering interface --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX_SPEXISELLOWERING_H
#define LLVM_LIB_TARGET_SPEX_SPEXISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SPEXTargetMachine;
class SPEXSubtarget;

namespace SPEXISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  CALL,
  RET,
  BR,
  BR_CC,
  SHL_I,
  SRL_I,
  SRA_I,
  LSTOP,
  LWAIT,
  LWAKE,
};
} // namespace SPEXISD

class SPEXTargetLowering : public TargetLowering {
  const SPEXSubtarget &ST;

public:
  explicit SPEXTargetLowering(const SPEXTargetMachine &TM,
                              const SPEXSubtarget &ST);

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  SDValue LowerBR(SDValue Chain, SDValue Dest, const SDLoc &DL,
                  SelectionDAG &DAG) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CC, bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CC, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerBR_CC(SDValue Chain, ISD::CondCode CC, SDValue LHS, SDValue RHS,
                     SDValue Dest, const SDLoc &DL, SelectionDAG &DAG) const;

  SDValue LowerShift(SDValue Op, SelectionDAG &DAG, unsigned TgtOpc) const;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  LegalizeKind getTypeConversion(LLVMContext &ctx, EVT VT);

  EVT getTypeToTransformTo(LLVMContext &ctx, EVT VT) const override;

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

private:
  SDValue lowerCallResult(SDValue Chain, SDValue InGlue, const SDLoc &DL,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;
};

} // namespace llvm

#endif
