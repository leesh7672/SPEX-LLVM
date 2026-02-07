//===-- SPEXISelDAGToDAG.cpp - DAG to DAG instruction selection --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEXISelDAGToDAG.h"
#include "SPEX.h"
#include "SPEXISelLowering.h"
#include "SPEXSubtarget.h"
#include "SPEXTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "spex-isel"
#define PASS_NAME "SPEX DAG->DAG Pattern Instruction Selection"

namespace {
class SPEXDAGToDAGISel final : public SelectionDAGISel {
  const SPEXSubtarget *Subtarget = nullptr;

public:
  explicit SPEXDAGToDAGISel(SPEXTargetMachine &TM) : SelectionDAGISel(TM) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SPEXSubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool SelectAddrRR(SDValue Addr, SDValue &Base, SDValue &Index);
  void Select(SDNode *Node) override;

#define GET_DAGISEL_DECL
#include "SPEXGenDAGISel.inc"
};

class SPEXDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  SPEXDAGToDAGISelLegacy(SPEXTargetMachine &TM)
      : SelectionDAGISelLegacy(ID, std::make_unique<SPEXDAGToDAGISel>(TM)) {}
};
} // end anonymous namespace

char SPEXDAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(SPEXDAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

#define GET_DAGISEL_BODY SPEXDAGToDAGISel
#include "SPEXGenDAGISel.inc"

bool SPEXDAGToDAGISel::SelectAddr(SDValue Addr, SDValue &Base,
                                  SDValue &Offset) {
  SDLoc DL(Addr);

  // Materialize absolute constant addresses into a register.
  // IMPORTANT: Use a *Target* constant so the rest of SelectionDAG/MI
  // emission treats it as an immediate operand and not a value that needs
  // additional legalization.
  if (auto *CN = dyn_cast<ConstantSDNode>(Addr)) {
    SDValue Imm = CurDAG->getTargetConstant(CN->getSExtValue(), DL, MVT::i64);
    SDNode *Li = CurDAG->getMachineNode(SPEX::PSEUDO_LI64, DL, MVT::i64, Imm);
    Base = SDValue(Li, 0);
    Offset = CurDAG->getTargetConstant(0, DL, MVT::i32);
    return true;
  }

  if (auto *FI = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i64);
    Offset = CurDAG->getTargetConstant(0, DL, MVT::i32);
    return true;
  }

  if (Addr.getOpcode() == ISD::TargetGlobalAddress ||
      Addr.getOpcode() == ISD::TargetExternalSymbol) {
    SDNode *Li = CurDAG->getMachineNode(SPEX::PSEUDO_LI64, DL, MVT::i64, Addr);
    Base = SDValue(Li, 0);
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

bool SPEXDAGToDAGISel::SelectAddrRR(SDValue Addr, SDValue &Base,
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

void SPEXDAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
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
        case SPEX::PSEUDO_CALL:
        case SPEX::CALL:
        case SPEX::CALL32:
        case SPEX::CALL64:
          if (UI->getOperandNo() == 0)
            return;
          break;
        default:
          break;
        }
        continue;
      }
      if (UseN->getOpcode() == SPEXISD::CALL &&
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
    SDNode *Res = CurDAG->getMachineNode(SPEX::PSEUDO_LI64, DL, MVT::i64, Addr);
    ReplaceNode(Node, Res);
    return;
  }

  case ISD::Constant: {
    SDLoc DL(Node);
    EVT VT = Node->getValueType(0);
    if (!VT.isInteger())
      break;

    unsigned Bits = VT.getSizeInBits();
    unsigned Opc = 0;

    MVT ImmVT = MVT::i32;

    switch (Bits) {
    case 8:
      Opc = SPEX::PSEUDO_LI8;
      ImmVT = MVT::i8;
      break;
    case 16:
      Opc = SPEX::PSEUDO_LI16;
      ImmVT = MVT::i16;
      break;
    case 32:
      Opc = SPEX::PSEUDO_LI32;
      ImmVT = MVT::i32;
      break;
    case 64:
      Opc = SPEX::PSEUDO_LI64;
      ImmVT = MVT::i64;
      break;
    default:
      return;
    }

    auto *CN = cast<ConstantSDNode>(Node);

    int64_t V = CN->getZExtValue();

    if (Bits == 8)
      V &= 0xFFull;
    if (Bits == 16)
      V &= 0xFFFFull;
    if (Bits == 32)
      V &= 0xFFFFFFFFull;

    SDValue Imm = CurDAG->getTargetConstant(V, DL, ImmVT);

    SDNode *Res = CurDAG->getMachineNode(Opc, DL, VT, Imm);
    ReplaceNode(Node, Res);
    return;
  }

  case SPEXISD::SHL_I:
  case SPEXISD::SRL_I:
  case SPEXISD::SRA_I: {
    EVT VT = Node->getValueType(0);
    unsigned Bits = VT.getSizeInBits();
    if (Bits != 32 && Bits != 64)
      break;

    unsigned Opcode = Node->getOpcode();
    const unsigned PseudoOpc =
        (Opcode == SPEXISD::SHL_I)
            ? (Bits == 32 ? SPEX::PSEUDO_SHL32ri : SPEX::PSEUDO_SHL64ri)
        : (Opcode == SPEXISD::SRL_I)
            ? (Bits == 32 ? SPEX::PSEUDO_SRL32ri : SPEX::PSEUDO_SRL64ri)
            : (Bits == 32 ? SPEX::PSEUDO_SRA32ri : SPEX::PSEUDO_SRA64ri);

    auto *AmtC = dyn_cast<ConstantSDNode>(Node->getOperand(1));
    if (!AmtC)
      break;
    SDValue Amt = CurDAG->getTargetConstant(AmtC->getZExtValue(), DL, MVT::i32);

    SDNode *Res =
        CurDAG->getMachineNode(PseudoOpc, DL, VT, Node->getOperand(0), Amt);
    ReplaceNode(Node, Res);
    return;
  }

  case SPEXISD::CALL: {
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

    // Operand order for call machine nodes must place Chain/Glue last.
    // Normal operands (callee, regmask, arg regs) go first.
    SmallVector<SDValue, 8> Ops;
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

    // Select the concrete call form.
    // - Direct symbol/addr calls use the absolute-address immediate form.
    // - Everything else is an indirect call through a register value.
    unsigned CallOpc = SPEX::CALL64;
    if (Callee.getOpcode() == ISD::TargetGlobalAddress ||
        Callee.getOpcode() == ISD::TargetExternalSymbol ||
        Callee.getOpcode() == ISD::TargetConstantPool ||
        Callee.getOpcode() == ISD::TargetBlockAddress)
      CallOpc = SPEX::CALL;

    if (Glue.getNode())
      Ops.push_back(Glue);

    SDNode *Call =
        CurDAG->getMachineNode(CallOpc, DL, MVT::Other, MVT::Glue, Ops);
    ReplaceNode(Node, Call);
    return;
  }

  case SPEXISD::BR: {
    SDValue Chain = Node->getOperand(0);
    SDValue Dest = Node->getOperand(1); // BasicBlock
    SDValue Ops[] = {Dest, Chain};
    SDNode *Br = CurDAG->getMachineNode(SPEX::JMP, DL, MVT::Other, Ops);
    ReplaceNode(Node, Br);
    return;
  }
  case SPEXISD::BR_CC: {
    SDLoc DL(Node);

    unsigned I = 0;
    SDValue InGlue;
    if (Node->getOperand(0).getValueType() == MVT::Glue) {
      InGlue = Node->getOperand(0);
      I = 1;
    }

    SDValue Chain = Node->getOperand(I + 0);
    SDValue LHS = Node->getOperand(I + 1);
    SDValue RHS = Node->getOperand(I + 2);
    auto CC = cast<CondCodeSDNode>(Node->getOperand(I + 3))->get();
    SDValue Dest = Node->getOperand(I + 4);

    SDValue CopyTo = CurDAG->getCopyToReg(Chain, DL, SPEX::RX, LHS, InGlue);
    SDValue CopyCh = CopyTo.getValue(0);
    SDValue CopyGlue = CopyTo.getValue(1);

    unsigned LBits = LHS.getValueType().getSizeInBits();
    bool RHSIsImm = isa<ConstantSDNode>(RHS);

    auto pickCmpR = [&] {
      switch (LBits) {
      case 8:
        return SPEX::CMP8_R;
      case 16:
        return SPEX::CMP16_R;
      case 32:
        return SPEX::CMP32_R;
      case 64:
        return SPEX::CMP64_R;
      default:
        llvm_unreachable("bad cmp width");
      }
    };

    auto pickCmpI = [&](int64_t Imm) {
      switch (LBits) {
      case 8:
        return SPEX::CMP8_I32;
      case 16:
        return SPEX::CMP16_I32;
      case 32:
        return SPEX::CMP32_I32;
      case 64:
        return isInt<32>(Imm) ? SPEX::CMP64_I32 : SPEX::CMP64_I64;
      default:
        llvm_unreachable("bad cmp width");
      }
    };

    unsigned CmpOpc = 0;
    SmallVector<SDValue, 6> CmpOps;

    if (RHSIsImm) {
      CmpOpc = pickCmpI(LHS.getValueType().getSizeInBits());
    } else {
      CmpOpc = pickCmpR();
    }

    CmpOps.push_back(CopyCh);
    CmpOps.push_back(RHS);
    CmpOps.push_back(CopyGlue);

    SDNode *CmpN =
        CurDAG->getMachineNode(CmpOpc, DL, MVT::Other, MVT::Glue, CmpOps);
    SDValue CmpCh = SDValue(CmpN, 0);
    SDValue CmpGlue = SDValue(CmpN, 1);

    unsigned BccOpc;
    switch (CC) {
    case ISD::SETEQ:
      BccOpc = SPEX::BCC_eq_I64;
      break;
    case ISD::SETNE:
      BccOpc = SPEX::BCC_ne_I64;
      break;
    case ISD::SETLT:
      BccOpc = SPEX::BCC_lt_I64;
      break;
    case ISD::SETULT:
      BccOpc = SPEX::BCC_ltu_I64;
      break;
    case ISD::SETLE:
      BccOpc = SPEX::BCC_le_I64;
      break;
    case ISD::SETULE:
      BccOpc = SPEX::BCC_leu_I64;
      break;
    case ISD::SETGT:
      BccOpc = SPEX::BCC_gt_I64;
      break;
    case ISD::SETUGT:
      BccOpc = SPEX::BCC_gtu_I64;
      break;
    case ISD::SETGE:
      BccOpc = SPEX::BCC_ge_I64;
      break;
    case ISD::SETUGE:
      BccOpc = SPEX::BCC_geu_I64;
      break;
    default:
      report_fatal_error("SPEX: unsupported branch condition");
      break;
    }

    SmallVector<SDValue, 4> BrOps;

    BrOps.push_back(CmpCh);
    BrOps.push_back(Dest);
    BrOps.push_back(CmpGlue);

    SDNode *BrN = CurDAG->getMachineNode(BccOpc, DL, MVT::Other, CopyCh, Dest);
    ReplaceNode(Node, BrN);

    return;
  }
  case ISD::SIGN_EXTEND_INREG: {
    SDLoc DL(Node);

    EVT OutVT = Node->getValueType(0);
    EVT InVT = cast<VTSDNode>(Node->getOperand(1))->getVT();

    if (!OutVT.isSimple() || !InVT.isSimple() || !OutVT.isInteger() ||
        !InVT.isInteger())
      break;

    unsigned OutBits = OutVT.getSimpleVT().getSizeInBits();
    unsigned InBits = InVT.getSimpleVT().getSizeInBits();
    if (InBits >= OutBits)
      break;

    unsigned ShAmt = OutBits - InBits;

    SDValue Src = Node->getOperand(0);
    SDValue Imm = CurDAG->getTargetConstant(ShAmt, DL, MVT::i32);

    unsigned MovToRX = (OutBits == 32) ? SPEX::MOVMOV32 : SPEX::MOVMOV64;
    unsigned MovFromRX = (OutBits == 32) ? SPEX::MOVMOV32_R : SPEX::MOVMOV64_R;
    unsigned ShlOpc = (OutBits == 32) ? SPEX::SHL32 : SPEX::SHL64;
    unsigned SraOpc = (OutBits == 32) ? SPEX::SAR32 : SPEX::SAR64;

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
  case SPEXISD::RET: {
    SDLoc DL(Node);

    unsigned Opc = SPEX::RET;
    SDValue Chain = Node->getOperand(0);
    SDValue Glue;

    if (Node->getNumOperands() >= 2 &&
        Node->getOperand(1).getValueType() == MVT::Glue) {
      Glue = Node->getOperand(1);
      Opc = SPEX::RET_R0;
    }

    SmallVector<SDValue, 2> Ops;
    Ops.push_back(Chain);
    if (Glue.getNode()) {
      Ops.push_back(Glue);
    }

    SDNode *Ret = CurDAG->getMachineNode(Opc, DL, MVT::Other, Ops);
    ReplaceNode(Node, Ret);
    return;
  }
  case SPEXISD::LSTOP: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX::LSTOP, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  case SPEXISD::LWAIT: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX::LWAIT, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  case SPEXISD::LWAKE: {
    SDValue Chain = Node->getOperand(0);
    SDNode *N = CurDAG->getMachineNode(SPEX::LWAKE, DL, MVT::Other, Chain);
    ReplaceNode(Node, N);
    return;
  }
  }

  SelectCode(Node);
}

FunctionPass *llvm::createSPEXISelDag(SPEXTargetMachine &TM) {
  return new SPEXDAGToDAGISelLegacy(TM);
}

#undef DEBUG_TYPE
#undef PASS_NAME
