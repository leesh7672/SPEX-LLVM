//===-- SPEX64ISelLowering.h - SPEX64 lowering interface --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPEX64_SPEX64ISELLOWERING_H
#define LLVM_LIB_TARGET_SPEX64_SPEX64ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SPEX64TargetMachine;
class SPEX64Subtarget;

namespace SPEX64ISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  CALL,
  RET,
  BR,
  BR_CC,
};
} // namespace SPEX64ISD

class SPEX64TargetLowering : public TargetLowering {
  const SPEX64Subtarget &ST;

public:
  explicit SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                const SPEX64Subtarget &ST);

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
                     SDValue Dest, const SDLoc &DL,
                     SelectionDAG &DAG) const;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

private:
  SDValue lowerCallResult(SDValue Chain, SDValue InGlue, const SDLoc &DL,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const;
};

} // namespace llvm

#endif
