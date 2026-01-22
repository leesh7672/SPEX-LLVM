#include "SPEX64ISelLowering.h"
#include "SPEX64.h"
#include "SPEX64Subtarget.h"
#include "SPEX64TargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

static const unsigned ArgRegs[] = {
    SPEX64::R0, SPEX64::R1, SPEX64::R2, SPEX64::R3, SPEX64::R4, SPEX64::R5,
};
static constexpr unsigned NumArgRegs =
    sizeof(ArgRegs) / sizeof(ArgRegs[0]);

SPEX64TargetLowering::SPEX64TargetLowering(const SPEX64TargetMachine &TM,
                                           const SPEX64Subtarget &ST)
    : TargetLowering(TM, ST), ST(ST) {
  addRegisterClass(MVT::i64, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i32, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i16, &SPEX64::GPRRegClass);
  addRegisterClass(MVT::i8, &SPEX64::GPRRegClass);

  computeRegisterProperties(ST.getRegisterInfo());

  setOperationAction(ISD::Constant, MVT::i8, Promote);
  setOperationAction(ISD::Constant, MVT::i16, Promote);
  setOperationAction(ISD::BR, MVT::Other, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::i64, Custom);

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
  case ISD::BR:
    return LowerBR(Op.getOperand(0), Op.getOperand(1), SDLoc(Op), DAG);
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

SDValue SPEX64TargetLowering::LowerBR(SDValue Chain, SDValue Dest,
                                      const SDLoc &DL,
                                      SelectionDAG &DAG) const {
  return DAG.getNode(SPEX64ISD::BR, DL, MVT::Other, Chain, Dest);
}

SDValue SPEX64TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                        SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc DL(CLI.DL);
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;

  if (CLI.IsTailCall)
    report_fatal_error("SPEX64: tail calls are not supported");

  if (CallConv != CallingConv::C && CallConv != CallingConv::Fast)
    report_fatal_error("SPEX64: unsupported calling convention");

  if (!CLI.Outs.empty())
    report_fatal_error("SPEX64: variadic or struct arguments not supported");

  unsigned RegIdx = 0;
  for (SDValue Arg : CLI.OutVals) {
    if (RegIdx >= NumArgRegs)
      report_fatal_error("SPEX64: too many call arguments");
    SDValue Copy = DAG.getCopyToReg(Chain, DL, ArgRegs[RegIdx], Arg);
    Chain = Copy.getValue(0);
    ++RegIdx;
  }

  SDValue Target;
  switch (Callee.getOpcode()) {
  case ISD::TargetGlobalAddress:
    Target = Callee;
    break;
  case ISD::GlobalAddress: {
    auto *GA = cast<GlobalAddressSDNode>(Callee);
    Target = DAG.getTargetGlobalAddress(GA->getGlobal(), 0, MVT::i64);
    break;
  }
  case ISD::ExternalSymbol: {
    auto *ES = cast<ExternalSymbolSDNode>(Callee);
    Target = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i64);
    break;
  }
  default:
    if (Callee.getValueType() != MVT::i64)
      Target = DAG.getNode(ISD::ANY_EXTEND, DL, MVT::i64, Callee);
    else
      Target = Callee;
    break;
  }

  SDNode *Call =
      DAG.getMachineNode(SPEX64::CALL64, DL, MVT::Other, Chain, Target);
  Chain = SDValue(Call, 0);

  return lowerCallResult(Chain, DL, CLI.Ins, DAG, InVals);
}

SDValue SPEX64TargetLowering::lowerCallResult(
    SDValue Chain, const SDLoc &DL, const SmallVectorImpl<ISD::InputArg> &Ins,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  if (Ins.empty())
    return Chain;
  if (Ins.size() > 1)
    report_fatal_error("SPEX64: multiple return values not supported");

  SDValue Copy =
      DAG.getCopyFromReg(Chain, DL, SPEX64::RX, MVT::i64, Chain.getValue(1));
  Chain = Copy.getValue(1);
  SDValue Val = Copy;
  if (Ins[0].VT != MVT::i64)
    Val = DAG.getNode(ISD::TRUNCATE, DL, Ins[0].VT, Val);
  InVals.push_back(Val);

  return Chain;
}
