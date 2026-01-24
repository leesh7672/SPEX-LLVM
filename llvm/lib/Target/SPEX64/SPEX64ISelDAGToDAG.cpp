//===-- SPEX64ISelDAGToDAG.cpp - DAG to DAG instruction selection --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64ISelDAGToDAG.h"
#include "SPEX64.h"
#include "SPEX64ISelLowering.h"
#include "SPEX64Subtarget.h"
#include "SPEX64TargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "spex64-isel"
#define PASS_NAME "SPEX64 DAG->DAG Pattern Instruction Selection"

namespace {
class SPEX64DAGToDAGISel final : public SelectionDAGISel {
  const SPEX64Subtarget *Subtarget = nullptr;

public:
  explicit SPEX64DAGToDAGISel(SPEX64TargetMachine &TM) : SelectionDAGISel(TM) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SPEX64Subtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool SelectAddrRR(SDValue Addr, SDValue &Base, SDValue &Index);
  void Select(SDNode *Node) override;

#define GET_DAGISEL_DECL
#include "SPEX64GenDAGISel.inc"
};

class SPEX64DAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  SPEX64DAGToDAGISelLegacy(SPEX64TargetMachine &TM)
      : SelectionDAGISelLegacy(ID, std::make_unique<SPEX64DAGToDAGISel>(TM)) {}
};
} // end anonymous namespace

char SPEX64DAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(SPEX64DAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

#define GET_DAGISEL_BODY SPEX64DAGToDAGISel
#include "SPEX64GenDAGISel.inc"

bool SPEX64DAGToDAGISel::SelectAddr(SDValue Addr, SDValue &Base,
                                    SDValue &Offset) {
  SDLoc DL(Addr);

  if (auto *FI = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i64);
    Offset = CurDAG->getTargetConstant(0, DL, MVT::i32);
    return true;
  }

  if (Addr.getOpcode() == ISD::ADD || Addr.getOpcode() == ISD::SUB) {
    SDValue LHS = Addr.getOperand(0);
    SDValue RHS = Addr.getOperand(1);

    if (auto *CN = dyn_cast<ConstantSDNode>(RHS)) {
      int64_t Off = CN->getSExtValue();
      if (Addr.getOpcode() == ISD::SUB)
        Off = -Off;
      if (isInt<32>(Off)) {
        Base = LHS;
        Offset = CurDAG->getTargetConstant(Off, DL, MVT::i32);
        return true;
      }
    }
  }

  Base = Addr;
  Offset = CurDAG->getTargetConstant(0, DL, MVT::i32);
  return true;
}

bool SPEX64DAGToDAGISel::SelectAddrRR(SDValue Addr, SDValue &Base,
                                      SDValue &Index) {
  if (Addr.getOpcode() == ISD::ADD || Addr.getOpcode() == ISD::SUB) {
    SDValue LHS = Addr.getOperand(0);
    SDValue RHS = Addr.getOperand(1);
    if (!isa<ConstantSDNode>(RHS)) {
      Base = LHS;
      Index = RHS;
      return true;
    }
  }
  return false;
}

void SPEX64DAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }

  SDLoc DL(Node);

  switch (Node->getOpcode()) {
  case SPEX64ISD::CALL: {
    SDValue Chain = Node->getOperand(0);
    SDValue Callee = Node->getOperand(1);
    SDValue Glue;

    SmallVector<SDValue, 8> Ops;
    for (unsigned I = 1, E = Node->getNumOperands(); I != E; ++I) {
      SDValue Op = Node->getOperand(I);
      if (Op.getValueType() == MVT::Glue) {
        Glue = Op;
        break;
      }
      Ops.push_back(Op);
    }

    Ops.push_back(Chain);

    unsigned CallOpc = SPEX64::CALL;
    if (Callee.getOpcode() == ISD::Register)
      CallOpc = SPEX64::CALLR;

    if (Glue.getNode())
      Ops.push_back(Glue);

    SDNode *Call =
        CurDAG->getMachineNode(CallOpc, DL, MVT::Other, MVT::Glue, Ops);
    ReplaceNode(Node, Call);
    return;
  }

  case SPEX64ISD::BR: {
    SDValue Chain = Node->getOperand(0);
    SDValue Dest = Node->getOperand(1); // BasicBlock
    SDValue Ops[] = {Dest, Chain};
    SDNode *Br = CurDAG->getMachineNode(SPEX64::JMP32, DL, MVT::Other, Ops);
    ReplaceNode(Node, Br);
    return;
  }

  case SPEX64ISD::BR_CC: {
    SDLoc DL(Node);

    SDValue Chain = Node->getOperand(0);
    SDValue LHS = Node->getOperand(1);
    SDValue RHS = Node->getOperand(2);
    auto CC = cast<CondCodeSDNode>(Node->getOperand(3))->get();
    SDValue Dest = Node->getOperand(4);

    auto PickSZ2FromVT = [](EVT VT) -> unsigned {
      if (VT == MVT::i8)
        return 0b00;
      if (VT == MVT::i16)
        return 0b01;
      if (VT == MVT::i32)
        return 0b10;
      return 0b11; // i64 default
    };

    EVT CmpVT = LHS.getValueType();
    if (LHS.getOpcode() == ISD::SIGN_EXTEND_INREG) {
      if (auto *VTN = dyn_cast<VTSDNode>(LHS.getOperand(1))) {
        CmpVT = VTN->getVT();
      }
    }

    const unsigned SZ2 = PickSZ2FromVT(CmpVT);
    const bool Is64Imm = (SZ2 == 0b11);

    unsigned MovOpc = 0;
    switch (SZ2) {
    case 0b00:
      MovOpc = SPEX64::MOVMOV8;
      break;
    case 0b01:
      MovOpc = SPEX64::MOVMOV16;
      break;
    case 0b10:
      MovOpc = SPEX64::MOVMOV32;
      break;
    case 0b11:
      MovOpc = SPEX64::MOVMOV64;
      break;
    }

    SDNode *MovN = CurDAG->getMachineNode(MovOpc, DL, MVT::Glue, LHS);
    SDValue MovGlue(MovN, 0);

    unsigned CmpOpc = 0;
    SmallVector<SDValue, 4> CmpOps;

    auto PickCmpROpc = [&](unsigned sz2) -> unsigned {
      switch (sz2) {
      case 0b00:
        return SPEX64::CMP8_R;
      case 0b01:
        return SPEX64::CMP16_R;
      case 0b10:
        return SPEX64::CMP32_R;
      default:
        return SPEX64::CMP64_R;
      }
    };
    auto PickCmpI32Opc = [&](unsigned sz2) -> unsigned {
      switch (sz2) {
      case 0b00:
        return SPEX64::CMP8_I32;
      case 0b01:
        return SPEX64::CMP16_I32;
      case 0b10:
        return SPEX64::CMP32_I32;
      default:
        return SPEX64::CMP64_I32;
      }
    };
    auto PickCmpI64Opc = [&](unsigned sz2) -> unsigned {
      switch (sz2) {
      case 0b00:
        return SPEX64::CMP8_I64;
      case 0b01:
        return SPEX64::CMP16_I64;
      case 0b10:
        return SPEX64::CMP32_I64;
      default:
        return SPEX64::CMP64_I64;
      }
    };

    if (auto *CN = dyn_cast<ConstantSDNode>(RHS)) {
      int64_t Imm = CN->getSExtValue();
      if (isInt<32>(Imm)) {
        CmpOpc = PickCmpI32Opc(SZ2);
        CmpOps.push_back(CurDAG->getTargetConstant(Imm, DL, MVT::i32));
      } else {
        CmpOpc = PickCmpI64Opc(SZ2);
        CmpOps.push_back(CurDAG->getTargetConstant(Imm, DL, MVT::i64));
      }
    } else {
      CmpOpc = PickCmpROpc(SZ2);
      CmpOps.push_back(RHS);
    }

    CmpOps.push_back(MovGlue);
    SDNode *CmpN = CurDAG->getMachineNode(CmpOpc, DL, MVT::Glue, CmpOps);
    SDValue CmpGlue(CmpN, 0);

    unsigned BccOpc = 0;
    auto PickBcc = [&](unsigned Opc32, unsigned Opc64) -> unsigned {
      return (SZ2 == 0b11) ? Opc64 : Opc32;
    };

    switch (CC) {
    case ISD::SETEQ:
      BccOpc = PickBcc(SPEX64::BCC_eq_32, SPEX64::BCC_eq_64);
      break;
    case ISD::SETNE:
      BccOpc = PickBcc(SPEX64::BCC_ne_32, SPEX64::BCC_ne_64);
      break;

    case ISD::SETLT:
      BccOpc = PickBcc(SPEX64::BCC_lt_32, SPEX64::BCC_lt_64);
      break;
    case ISD::SETLE:
      BccOpc = PickBcc(SPEX64::BCC_le_32, SPEX64::BCC_le_64);
      break;
    case ISD::SETGT:
      BccOpc = PickBcc(SPEX64::BCC_gt_32, SPEX64::BCC_gt_64);
      break;
    case ISD::SETGE:
      BccOpc = PickBcc(SPEX64::BCC_ge_32, SPEX64::BCC_ge_64);
      break;

    case ISD::SETULT:
      BccOpc = PickBcc(SPEX64::BCC_ltu_32, SPEX64::BCC_ltu_64);
      break;
    case ISD::SETULE:
      BccOpc = PickBcc(SPEX64::BCC_leu_32, SPEX64::BCC_leu_64);
      break;
    case ISD::SETUGT:
      BccOpc = PickBcc(SPEX64::BCC_gtu_32, SPEX64::BCC_gtu_64);
      break;
    case ISD::SETUGE:
      BccOpc = PickBcc(SPEX64::BCC_geu_32, SPEX64::BCC_geu_64);
      break;

    default:
      BccOpc = PickBcc(SPEX64::BCC_ne_32, SPEX64::BCC_ne_64);
      break;
    }

    SmallVector<SDValue, 3> BrOps;
    BrOps.push_back(Chain);
    BrOps.push_back(Dest);
    BrOps.push_back(CmpGlue);

    SDNode *BrN = CurDAG->getMachineNode(BccOpc, DL, MVT::Other, BrOps);
    ReplaceNode(Node, BrN);
    return;
  }

  case SPEX64ISD::RET: {
    SDValue Chain = Node->getOperand(0);
    if (Node->getNumOperands() > 1) {
      SDValue Glue = Node->getOperand(1);
      SDValue Ops[] = {Chain, Glue};
      SDNode *Ret = CurDAG->getMachineNode(SPEX64::RET, DL, MVT::Other, Ops);
      ReplaceNode(Node, Ret);
      return;
    }
    SDNode *Ret = CurDAG->getMachineNode(SPEX64::RET, DL, MVT::Other, Chain);
    ReplaceNode(Node, Ret);
    return;
  }
  case SPEX64ISD::LSTOP: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX64::LSTOP, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  case SPEX64ISD::LWAIT: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX64::LWAIT, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  case SPEX64ISD::LWAKE: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX64::LWAKE, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  }

  SelectCode(Node);
}

FunctionPass *llvm::createSPEX64ISelDag(SPEX64TargetMachine &TM) {
  return new SPEX64DAGToDAGISelLegacy(TM);
}

#undef DEBUG_TYPE
#undef PASS_NAME
