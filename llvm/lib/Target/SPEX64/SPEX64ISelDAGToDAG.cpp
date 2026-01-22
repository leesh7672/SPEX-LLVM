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
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "spex64-isel"
#define PASS_NAME "SPEX64 DAG->DAG Pattern Instruction Selection"

namespace {
class SPEX64DAGToDAGISel final : public SelectionDAGISel {
  const SPEX64Subtarget *Subtarget = nullptr;

public:
  explicit SPEX64DAGToDAGISel(SPEX64TargetMachine &TM)
      : SelectionDAGISel(TM) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SPEX64Subtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  void Select(SDNode *Node) override;
};
} // end anonymous namespace

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

void SPEX64DAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }

  SDLoc DL(Node);

  switch (Node->getOpcode()) {
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
  case SPEX64ISD::BR_CC: {
    SDValue Chain = Node->getOperand(0);
    SDValue LHS = Node->getOperand(1);
    SDValue RHS = Node->getOperand(2);
    SDValue CC = Node->getOperand(3);
    SDValue Dest = Node->getOperand(4);

    auto *CCNode = cast<CondCodeSDNode>(CC);
    SDValue CCVal = CurDAG->getTargetConstant(CCNode->get(), DL, MVT::i32);

    unsigned BrOpc = LHS.getValueType() == MVT::i32
                         ? SPEX64::PSEUDO_BR_CC32
                         : SPEX64::PSEUDO_BR_CC64;
    SDValue Ops[] = {Chain, LHS, RHS, Dest, CCVal};
    SDNode *Br = CurDAG->getMachineNode(BrOpc, DL, MVT::Other, Ops);
    ReplaceNode(Node, Br);
    return;
  }
  default:
    break;
  }

  SelectCode(Node);
}

FunctionPass *llvm::createSPEX64ISelDag(SPEX64TargetMachine &TM) {
  return new SPEX64DAGToDAGISel(TM);
}

#undef DEBUG_TYPE
#undef PASS_NAME
