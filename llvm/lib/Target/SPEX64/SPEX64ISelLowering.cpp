#include "SPEX64ISelLowering.h"
#include "SPEX64.h"
#include "SPEX64Subtarget.h"
#include "SPEX64TargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

SPEX64TargetLowering::SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                           const SPEX64Subtarget &ST)
    : TargetLowering(TM, ST), ST(ST) {
  addRegisterClass(MVT::i64, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i32, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i16, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i8, &SPEX64::GPRRegClass);

  computeRegisterProperties(ST.getRegisterInfo());

  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::i64, Custom);
  setOperationAction(ISD::Constant, MVT::i8, Promote);
  setOperationAction(ISD::Constant, MVT::i16, Promote);

  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i64, Expand);

  setBooleanContents(ZeroOrOneBooleanContent);
}

SDValue SPEX64TargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BR_CC:
    return LowerBR_CC(Op.getOperand(0),
                      cast<CondCodeSDNode>(Op.getOperand(1))->get(),
                      Op.getOperand(2), Op.getOperand(3), Op.getOperand(4),
                      SDLoc(Op), DAG);
  default:
    break;
  }
  return SDValue();
}

SDValue SPEX64TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID, bool,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  if (!Ins.empty())
    report_fatal_error("SPEX64: formal arguments not supported yet");
  return Chain;
}

SDValue SPEX64TargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID, bool,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
    SelectionDAG &DAG) const {
  if (Outs.size() > 1)
    report_fatal_error("SPEX64: only one return value is supported");

  if (!OutVals.empty()) {
    SDValue RetVal = OutVals[0];
    if (RetVal.getValueType() != MVT::i64)
      RetVal = DAG.getNode(ISD::ANY_EXTEND, DL, MVT::i64, RetVal);
    SDValue Glue;
    Chain = DAG.getCopyToReg(Chain, DL, SPEX64::RX, RetVal, Glue);
    Glue = Chain.getValue(1);
    return DAG.getNode(SPEX64ISD::RET, DL, MVT::Other, Chain, Glue);
  }

  return DAG.getNode(SPEX64ISD::RET, DL, MVT::Other, Chain);
}

SDValue SPEX64TargetLowering::LowerBR_CC(SDValue Chain, ISD::CondCode CC,
                                         SDValue LHS, SDValue RHS,
                                         SDValue Dest, const SDLoc &DL,
                                         SelectionDAG &DAG) const {
  SDValue CCVal = DAG.getCondCode(CC);
  return DAG.getNode(SPEX64ISD::BR_CC, DL, MVT::Other, Chain, LHS, RHS, CCVal,
                     Dest);
}
