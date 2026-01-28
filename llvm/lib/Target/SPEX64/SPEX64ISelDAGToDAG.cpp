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
    Offset = CurDAG->getConstant(0, DL, MVT::i32);
    return true;
  }

  if (isa<ConstantSDNode>(Addr) ||
      Addr.getOpcode() == ISD::TargetGlobalAddress ||
      Addr.getOpcode() == ISD::TargetExternalSymbol) {
    SDNode *Li = CurDAG->getMachineNode(SPEX64::LILI64_64, DL, MVT::i64, Addr);
    Base = SDValue(Li, 0);
    Offset = CurDAG->getConstant(0, DL, MVT::i32);
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
        Offset = CurDAG->getConstant(Off, DL, MVT::i32);
        return true;
      }
    }
  }

  Base = Addr;
  Offset = CurDAG->getConstant(0, DL, MVT::i32);
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

  case ISD::GlobalAddress:
  case ISD::ExternalSymbol:
  case ISD::ConstantPool:
  case ISD::JumpTable:
  case ISD::BlockAddress:
  case ISD::TargetGlobalAddress:
  case ISD::TargetExternalSymbol:
  case ISD::TargetConstantPool:
  case ISD::TargetJumpTable:
  case ISD::TargetBlockAddress: {
    // If used as a direct call target, let the CALL selector handle it.
    for (auto UI = Node->use_begin(), UE = Node->use_end(); UI != UE; ++UI) {
      SDNode *UseN = UI->getUser();
      if (UseN->isMachineOpcode()) {
        switch (UseN->getMachineOpcode()) {
        case SPEX64::CALL:
        case SPEX64::CALLI:
        case SPEX64::CALL32:
        case SPEX64::CALL64:
          if (UI->getOperandNo() == 0)
            return;
          break;
        default:
          break;
        }
        continue;
      }
      if (UseN->getOpcode() == SPEX64ISD::CALL &&
          (UI->getOperandNo() == 0 || UI->getOperandNo() == 1))
        return;
    }
    SDValue Addr(Node, 0);
    switch (Node->getOpcode()) {
    case ISD::GlobalAddress: {
      auto *GA = cast<GlobalAddressSDNode>(Node);
      Addr = CurDAG->getTargetGlobalAddress(
          GA->getGlobal(), DL, MVT::i64, GA->getOffset(), GA->getTargetFlags());
      break;
    }
    case ISD::ExternalSymbol: {
      auto *ES = cast<ExternalSymbolSDNode>(Node);
      Addr = CurDAG->getTargetExternalSymbol(ES->getSymbol(), MVT::i64,
                                             ES->getTargetFlags());
      break;
    }
    case ISD::ConstantPool: {
      auto *CP = cast<ConstantPoolSDNode>(Node);
      Addr = CurDAG->getTargetConstantPool(CP->getConstVal(), MVT::i64,
                                           CP->getAlign(), CP->getOffset());
      break;
    }
    case ISD::JumpTable: {
      auto *JT = cast<JumpTableSDNode>(Node);
      Addr = CurDAG->getTargetJumpTable(JT->getIndex(), MVT::i64,
                                        JT->getTargetFlags());
      break;
    }
    case ISD::BlockAddress: {
      auto *BA = cast<BlockAddressSDNode>(Node);
      Addr =
          CurDAG->getTargetBlockAddress(BA->getBlockAddress(), MVT::i64,
                                        BA->getOffset(), BA->getTargetFlags());
      break;
    }
    default:
      break;
    }
    SDNode *Res =
        CurDAG->getMachineNode(SPEX64::PSEUDO_LI64, DL, MVT::i64, Addr);
    ReplaceNode(Node, Res);
    return;
  }

  case ISD::Constant: {
    EVT VT = Node->getValueType(0);
    if (!VT.isInteger())
      break;
    unsigned Bits = VT.getSizeInBits();
    unsigned Opc = 0;
    MVT ImmVT = MVT::i32;
    switch (Bits) {
    case 8:
      Opc = SPEX64::PSEUDO_LI8;
      break;
    case 16:
      Opc = SPEX64::PSEUDO_LI16;
      break;
    case 32:
      Opc = SPEX64::PSEUDO_LI32;
      break;
    case 64:
      Opc = SPEX64::PSEUDO_LI64;
      ImmVT = MVT::i64;
      break;
    default:
      break;
    }
    if (!Opc)
      break;
    auto *CN = cast<ConstantSDNode>(Node);
    SDValue Imm = CurDAG->getTargetConstant(CN->getSExtValue(), DL, ImmVT);
    SDNode *Res = CurDAG->getMachineNode(Opc, DL, VT, Imm);
    ReplaceNode(Node, Res);
    return;
  }

  case SPEX64ISD::SHL_I:
  case SPEX64ISD::SRL_I:
  case SPEX64ISD::SRA_I: {
    EVT VT = Node->getValueType(0);
    unsigned Bits = VT.getSizeInBits();
    if (Bits != 32 && Bits != 64)
      break;

    unsigned Opcode = Node->getOpcode();
    const unsigned PseudoOpc =
        (Opcode == SPEX64ISD::SHL_I)
            ? (Bits == 32 ? SPEX64::PSEUDO_SHL32ri : SPEX64::PSEUDO_SHL64ri)
        : (Opcode == SPEX64ISD::SRL_I)
            ? (Bits == 32 ? SPEX64::PSEUDO_SRL32ri : SPEX64::PSEUDO_SRL64ri)
            : (Bits == 32 ? SPEX64::PSEUDO_SRA32ri : SPEX64::PSEUDO_SRA64ri);

    auto *AmtC = dyn_cast<ConstantSDNode>(Node->getOperand(1));
    if (!AmtC)
      break;
    SDValue Amt = CurDAG->getTargetConstant(AmtC->getZExtValue(), DL, MVT::i32);

    SDNode *Res =
        CurDAG->getMachineNode(PseudoOpc, DL, VT, Node->getOperand(0), Amt);
    ReplaceNode(Node, Res);
    return;
  }

  case SPEX64ISD::CALL: {
    SDValue Chain = Node->getOperand(0);
    SDValue Callee = Node->getOperand(1);
    SDValue Glue;

    switch (Callee.getOpcode()) {
    case ISD::GlobalAddress: {
      auto *GA = cast<GlobalAddressSDNode>(Callee);
      Callee = CurDAG->getTargetGlobalAddress(
          GA->getGlobal(), DL, MVT::i64, GA->getOffset(), GA->getTargetFlags());
      break;
    }
    case ISD::ExternalSymbol: {
      auto *ES = cast<ExternalSymbolSDNode>(Callee);
      Callee = CurDAG->getTargetExternalSymbol(ES->getSymbol(), MVT::i64,
                                               ES->getTargetFlags());
      break;
    }
    case ISD::ConstantPool: {
      auto *CP = cast<ConstantPoolSDNode>(Callee);
      Callee = CurDAG->getTargetConstantPool(CP->getConstVal(), MVT::i64,
                                             CP->getAlign(), CP->getOffset());
      break;
    }
    case ISD::TargetGlobalAddress:
    case ISD::TargetExternalSymbol:
    case ISD::TargetConstantPool:
      break;
    default:
      break;
    }

    // Operand order for machine call nodes must begin with the Chain.
    // The callee (direct symbol or indirect register) follows, then any
    // additional operands such as the register mask and argument registers.
    SmallVector<SDValue, 8> Ops;
    Ops.push_back(Chain);
    Ops.push_back(Callee);

    for (unsigned I = 2, E = Node->getNumOperands(); I != E; ++I) {
      SDValue Op = Node->getOperand(I);
      if (Op.getValueType() == MVT::Glue) {
        Glue = Op;
        break;
      }
      Ops.push_back(Op);
    }

    Ops.push_back(Chain);

    unsigned CallOpc = SPEX64::CALLR;
    // Direct calls use the immediate form (CALL). Everything else is an
    // indirect call through a value (which will be selected into a register),
    // so must use CALLR to avoid losing the target and encoding an all-zero
    // immediate.
    if (Callee.getOpcode() == ISD::TargetGlobalAddress ||
        Callee.getOpcode() == ISD::TargetExternalSymbol ||
        Callee.getOpcode() == ISD::TargetConstantPool ||
        Callee.getOpcode() == ISD::TargetBlockAddress)
      CallOpc = SPEX64::CALLI;

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
    SDValue Chain = Node->getOperand(0);
    SDValue LHS = Node->getOperand(1);
    SDValue RHS = Node->getOperand(2);
    auto CC = cast<CondCodeSDNode>(Node->getOperand(3))->get();
    SDValue Dest = Node->getOperand(4);

    bool Is64 = (LHS.getValueType() == MVT::i64);

    // mov{32,64} rx, LHS
    unsigned MovOpc = Is64 ? SPEX64::MOVMOV64 : SPEX64::MOVMOV32;
    SDNode *MovN = CurDAG->getMachineNode(MovOpc, DL, MVT::Glue, LHS);
    SDValue MovGlue(MovN, 0);

    // cmp{32,64} rx, RHS (reg or imm)
    unsigned CmpOpc = 0;
    SmallVector<SDValue, 4> CmpOps;
    if (auto *CN = dyn_cast<ConstantSDNode>(RHS)) {
      int64_t Imm = CN->getSExtValue();
      if (Is64) {
        // Prefer I32 form when it fits.
        if (isInt<32>(Imm)) {
          CmpOpc = SPEX64::CMP64_I32;
          CmpOps.push_back(CurDAG->getTargetConstant(Imm, DL, MVT::i32));
        } else {
          CmpOpc = SPEX64::CMP64_I64;
          CmpOps.push_back(CurDAG->getTargetConstant(Imm, DL, MVT::i64));
        }
      } else {
        // i32 compare
        CmpOpc = SPEX64::CMP32_I32;
        CmpOps.push_back(CurDAG->getTargetConstant(Imm, DL, MVT::i32));
      }
    } else {
      CmpOpc = Is64 ? SPEX64::CMP64_R : SPEX64::CMP32_R;
      CmpOps.push_back(RHS);
    }
    CmpOps.push_back(MovGlue);
    SDNode *CmpN = CurDAG->getMachineNode(CmpOpc, DL, MVT::Glue, CmpOps);
    SDValue CmpGlue(CmpN, 0);

    // Select BCC opcode based on condition code.
    unsigned BccOpc = 0;
    switch (CC) {
    case ISD::SETEQ:
      BccOpc = Is64 ? SPEX64::BCC_eq_64 : SPEX64::BCC_eq_32;
      break;
    case ISD::SETNE:
      BccOpc = Is64 ? SPEX64::BCC_ne_64 : SPEX64::BCC_ne_32;
      break;
    case ISD::SETLT:
    case ISD::SETULT:
      BccOpc = Is64 ? SPEX64::BCC_lt_64 : SPEX64::BCC_lt_32;
      break;
    case ISD::SETLE:
    case ISD::SETULE:
      BccOpc = Is64 ? SPEX64::BCC_le_64 : SPEX64::BCC_le_32;
      break;
    case ISD::SETGT:
    case ISD::SETUGT:
      BccOpc = Is64 ? SPEX64::BCC_gt_64 : SPEX64::BCC_gt_32;
      break;
    case ISD::SETGE:
    case ISD::SETUGE:
      BccOpc = Is64 ? SPEX64::BCC_ge_64 : SPEX64::BCC_ge_32;
      break;
    default:
      // Fallback: treat as !=
      BccOpc = Is64 ? SPEX64::BCC_ne_64 : SPEX64::BCC_ne_32;
      break;
    }

    SmallVector<SDValue, 4> BrOps;
    BrOps.push_back(Dest);
    BrOps.push_back(Chain);
    BrOps.push_back(CmpGlue);
    SDNode *BrN = CurDAG->getMachineNode(BccOpc, DL, MVT::Other, BrOps);
    ReplaceNode(Node, BrN);
    return;
  }
  case ISD::SIGN_EXTEND_INREG: {
    SDLoc DL(Node);

    EVT OutVT = Node->getValueType(0);
    EVT InVT = cast<VTSDNode>(Node->getOperand(1))->getVT();

    // We only handle the common integer cases here (e.g. i64 sext_inreg(...,
    // i8)).
    if (!OutVT.isSimple() || !InVT.isSimple() || !OutVT.isInteger() ||
        !InVT.isInteger())
      break;

    unsigned OutBits = OutVT.getSimpleVT().getSizeInBits();
    unsigned InBits = InVT.getSimpleVT().getSizeInBits();
    if (InBits >= OutBits)
      break;

    // SPEX64 shift instructions operate on the implicit accumulator RX with an
    // immediate shift amount. Emit: RX = src; RX <<= shamt; RX sra= shamt;
    // dst = RX;
    unsigned ShAmt = OutBits - InBits;

    SDValue Src = Node->getOperand(0);
    SDValue Imm = CurDAG->getTargetConstant(ShAmt, DL, MVT::i32);

    unsigned MovToRX = (OutBits == 32) ? SPEX64::MOVMOV32 : SPEX64::MOVMOV64;
    unsigned MovFromRX =
        (OutBits == 32) ? SPEX64::MOVMOV32_R : SPEX64::MOVMOV64_R;
    unsigned ShlOpc = (OutBits == 32) ? SPEX64::SHL32 : SPEX64::SHL64;
    unsigned SraOpc = (OutBits == 32) ? SPEX64::SAR32 : SPEX64::SAR64;

    SDNode *MovN = CurDAG->getMachineNode(MovToRX, DL, MVT::Glue, Src);
    SDValue Glue(MovN, 0);

    SDNode *ShlN = CurDAG->getMachineNode(ShlOpc, DL, MVT::Glue, Imm, Glue);
    Glue = SDValue(ShlN, 0);

    SDNode *SraN = CurDAG->getMachineNode(SraOpc, DL, MVT::Glue, Imm, Glue);
    Glue = SDValue(SraN, 0);

    SDNode *OutN =
        CurDAG->getMachineNode(MovFromRX, DL, OutVT.getSimpleVT(), Glue);

    ReplaceNode(Node, OutN);
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
