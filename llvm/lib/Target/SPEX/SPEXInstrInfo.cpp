//===-- SPEXInstrInfo.cpp - SPEX instruction information --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include "llvm/Support/Debug.h"
//===----------------------------------------------------------------------===//

#include "SPEX.h"
#include "SPEXInstrInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SPEXGenInstrInfo.inc"

using namespace llvm;

SPEXInstrInfo::SPEXInstrInfo(const TargetSubtargetInfo &STI)
    : SPEXGenInstrInfo(STI, RI) {}

void SPEXInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, Register DestReg,
                                Register SrcReg, bool KillSrc,
                                bool RenamableDest, bool RenamableSrc) const {
  if (SPEX::GPRRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64))
        .addReg(SrcReg,
                getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64_R))
        .addReg(DestReg,
                RegState::Define | getRenamableRegState(RenamableDest));
    return;
  }

  if (DestReg == SPEX::RX && SPEX::GPRRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64))
        .addReg(SrcReg,
                getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
    return;
  }

  if (SPEX::GPRRegClass.contains(DestReg) && SrcReg == SPEX::RX) {
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64_R))
        .addReg(DestReg,
                RegState::Define | getRenamableRegState(RenamableDest));
    return;
  }

  report_fatal_error("SPEX copyPhysReg: unsupported register copy");
}

static unsigned getCondBranchOpcode(ISD::CondCode CC) {
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
  return BccOpc;
}

void SPEXInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  (void)TRI;
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  // We only support integer GPR spills right now.
  if (RC != &SPEX::GPRRegClass)
    report_fatal_error("SPEX: unsupported spill register class");

  // FrameIndex will be resolved later by eliminateFrameIndex.
  // Use the existing pseudo store so the post-RA expander can materialize
  // base into RX and pick ST size.
  unsigned Opc = SPEX::PSEUDO_ST64;
  BuildMI(MBB, MI, DL, get(Opc))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void SPEXInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         Register VReg, unsigned SubReg,
                                         MachineInstr::MIFlag Flags) const {
  (void)TRI;
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  if (RC != &SPEX::GPRRegClass)
    report_fatal_error("SPEX: unsupported reload register class");

  unsigned Opc = SPEX::PSEUDO_LDZ64;
  BuildMI(MBB, MI, DL, get(Opc), DestReg).addFrameIndex(FrameIndex).addImm(0);
}

bool SPEXInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  MachineBasicBlock::iterator I = MI.getIterator();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  case SPEX::ADJCALLSTACKDOWN:
  case SPEX::ADJCALLSTACKUP: {
    // SPEX has no call stack to adjust in this ABI; erase if zero.
    // LLVM may still emit these pseudos around calls.
    int64_t Amt1 = MI.getOperand(0).isImm() ? MI.getOperand(0).getImm() : 0;
    int64_t Amt2 = MI.getOperand(1).isImm() ? MI.getOperand(1).getImm() : 0;
    if (Amt1 == 0 && Amt2 == 0) {
      MI.eraseFromParent();
      return true;
    }
    report_fatal_error(
        "SPEX: ADJCALLSTACK* with non-zero amount is unsupported");
  }
  case SPEX::PSEUDO_CALL: {
    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(SPEX::CALL));
    for (const MachineOperand &MO : MI.operands())
      MIB.add(MO);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_CALLR: {
    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(SPEX::CALL64));
    for (const MachineOperand &MO : MI.operands())
      MIB.add(MO);
    MI.eraseFromParent();
    return true;
  }

  // Safety net: LI* only accepts an Imm/Expr operand. If something upstream
  // accidentally feeds a register into LI*, rewrite it into MOV* (rx <- r6).
  case SPEX::LILI8_32:
  case SPEX::LILI8_64:
  case SPEX::LILI16_32:
  case SPEX::LILI16_64:
  case SPEX::LILI32_32:
  case SPEX::LILI32_64:
  case SPEX::LILI64_32:
  case SPEX::LILI64_64: {
    if (MI.getNumOperands() >= 1 && MI.getOperand(0).isReg()) {
      Register Src = MI.getOperand(0).getReg();
      unsigned MovOpc = SPEX::MOVMOV64;
      switch (MI.getOpcode()) {
      case SPEX::LILI8_32:
      case SPEX::LILI8_64:
        MovOpc = SPEX::MOVMOV8;
        break;
      case SPEX::LILI16_32:
      case SPEX::LILI16_64:
        MovOpc = SPEX::MOVMOV16;
        break;
      case SPEX::LILI32_32:
      case SPEX::LILI32_64:
        MovOpc = SPEX::MOVMOV32;
        break;
      default:
        MovOpc = SPEX::MOVMOV64;
        break;
      }
      BuildMI(MBB, I, DL, get(MovOpc)).addReg(Src);
      MI.eraseFromParent();
      return true;
    }
    break;
  }

  case SPEX::PSEUDO_LI8: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV8)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV8_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX::LILI8_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV8_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LI16: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV16)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV16_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX::LILI16_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV16_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LI32: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV32)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV32_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX::LILI32_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LI64: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    unsigned LiOpc = SPEX::LILI64_64;
    if (SrcOp.isImm()) {
      int64_t Imm = SrcOp.getImm();
      if (isInt<32>(Imm))
        LiOpc = SPEX::LILI64_32;
    } else if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(LiOpc)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ADD32rr:
  case SPEX::PSEUDO_SUB32rr:
  case SPEX::PSEUDO_AND32rr:
  case SPEX::PSEUDO_OR32rr:
  case SPEX::PSEUDO_XOR32rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    Register RHS = MI.getOperand(2).getReg();
    unsigned AluOpc = SPEX::ADD32_R;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SUB32rr:
      AluOpc = SPEX::SUB32_R;
      break;
    case SPEX::PSEUDO_AND32rr:
      AluOpc = SPEX::AND32_R;
      break;
    case SPEX::PSEUDO_OR32rr:
      AluOpc = SPEX::OR32_R;
      break;
    case SPEX::PSEUDO_XOR32rr:
      AluOpc = SPEX::XOR32_R;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addReg(RHS);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ADD64rr:
  case SPEX::PSEUDO_SUB64rr:
  case SPEX::PSEUDO_AND64rr:
  case SPEX::PSEUDO_OR64rr:
  case SPEX::PSEUDO_XOR64rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    Register RHS = MI.getOperand(2).getReg();
    unsigned AluOpc = SPEX::ADD64_R;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SUB64rr:
      AluOpc = SPEX::SUB64_R;
      break;
    case SPEX::PSEUDO_AND64rr:
      AluOpc = SPEX::AND64_R;
      break;
    case SPEX::PSEUDO_OR64rr:
      AluOpc = SPEX::OR64_R;
      break;
    case SPEX::PSEUDO_XOR64rr:
      AluOpc = SPEX::XOR64_R;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addReg(RHS);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ADD32ri:
  case SPEX::PSEUDO_SUB32ri:
  case SPEX::PSEUDO_AND32ri:
  case SPEX::PSEUDO_OR32ri:
  case SPEX::PSEUDO_XOR32ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned AluOpc = SPEX::ADD32_I32;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SUB32ri:
      AluOpc = SPEX::SUB32_I32;
      break;
    case SPEX::PSEUDO_AND32ri:
      AluOpc = SPEX::AND32_I32;
      break;
    case SPEX::PSEUDO_OR32ri:
      AluOpc = SPEX::OR32_I32;
      break;
    case SPEX::PSEUDO_XOR32ri:
      AluOpc = SPEX::XOR32_I32;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addImm(Imm);
    //
    // NOTE:
    // Some toolchain paths have been observed to emit an illegal W0 (e.g.
    // 0x000000E8) at the point where we materialize RX -> GPR with MOVMOV32_R,
    // producing <unknown> in the final disassembly. As a safe workaround (until
    // the MOVMOV32_R encoding/ lowering issue is fully root-caused), use the
    // 64-bit RX->GPR move here.
    //
    // This preserves the low 32-bit result of the 32-bit ALU op in RX;
    // consumers that use the value as i32 will still see the correct value.
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ADD64ri:
  case SPEX::PSEUDO_SUB64ri:
  case SPEX::PSEUDO_AND64ri:
  case SPEX::PSEUDO_OR64ri:
  case SPEX::PSEUDO_XOR64ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    bool FitsI32 = isInt<32>(Imm);
    unsigned AluOpc = FitsI32 ? SPEX::ADD64_I32 : SPEX::ADD64_I64;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SUB64ri:
      AluOpc = FitsI32 ? SPEX::SUB64_I32 : SPEX::SUB64_I64;
      break;
    case SPEX::PSEUDO_AND64ri:
      AluOpc = FitsI32 ? SPEX::AND64_I32 : SPEX::AND64_I64;
      break;
    case SPEX::PSEUDO_OR64ri:
      AluOpc = FitsI32 ? SPEX::OR64_I32 : SPEX::OR64_I64;
      break;
    case SPEX::PSEUDO_XOR64ri:
      AluOpc = FitsI32 ? SPEX::XOR64_I32 : SPEX::XOR64_I64;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_SHL32ri:
  case SPEX::PSEUDO_SRL32ri:
  case SPEX::PSEUDO_SRA32ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned ShOpc = SPEX::SHL32;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SRL32ri:
      ShOpc = SPEX::SHR32;
      break;
    case SPEX::PSEUDO_SRA32ri:
      ShOpc = SPEX::SAR32;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32)).addReg(Src);
    BuildMI(MBB, I, DL, get(ShOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_SHL64ri:
  case SPEX::PSEUDO_SRL64ri:
  case SPEX::PSEUDO_SRA64ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned ShOpc = SPEX::SHL64;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_SRL64ri:
      ShOpc = SPEX::SHR64;
      break;
    case SPEX::PSEUDO_SRA64ri:
      ShOpc = SPEX::SAR64;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(ShOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LDZ8rr:
  case SPEX::PSEUDO_LDZ16rr:
  case SPEX::PSEUDO_LDZ32rr:
  case SPEX::PSEUDO_LDZ64rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX::ADD64_R)).addReg(Idx);

    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_LDZ8rr:
      LdOpc = SPEX::LDZ_R8;
      break;
    case SPEX::PSEUDO_LDZ16rr:
      LdOpc = SPEX::LDZ_R16;
      break;
    case SPEX::PSEUDO_LDZ32rr:
      LdOpc = SPEX::LDZ_R32;
      break;
    case SPEX::PSEUDO_LDZ64rr:
      LdOpc = SPEX::LDZ_R64;
      break;
    default:
      llvm_unreachable("Unexpected LDZrr pseudo");
    }

    BuildMI(MBB, I, DL, get(LdOpc)).addReg(Dst, RegState::Define);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LDZ8:
  case SPEX::PSEUDO_LDZ16:
  case SPEX::PSEUDO_LDZ32:
  case SPEX::PSEUDO_LDZ64: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_LDZ8:
      LdOpc = Off == 0 ? SPEX::LDZ_R8 : SPEX::LDZ_R8_I32;
      break;
    case SPEX::PSEUDO_LDZ16:
      LdOpc = Off == 0 ? SPEX::LDZ_R16 : SPEX::LDZ_R16_I32;
      break;
    case SPEX::PSEUDO_LDZ32:
      LdOpc = Off == 0 ? SPEX::LDZ_R32 : SPEX::LDZ_R32_I32;
      break;
    case SPEX::PSEUDO_LDZ64:
      LdOpc = Off == 0 ? SPEX::LDZ_R64 : SPEX::LDZ_R64_I32;
      break;
    default:
      llvm_unreachable("unexpected LDZ pseudo");
    }
    auto LdMI = BuildMI(MBB, I, DL, get(LdOpc), Dst);
    if (Off != 0)
      LdMI.addImm(Off);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LDS8rr:
  case SPEX::PSEUDO_LDS16rr:
  case SPEX::PSEUDO_LDS32rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX::ADD64_R)).addReg(Idx);

    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_LDS8rr:
      LdOpc = SPEX::LDS_R8;
      break;
    case SPEX::PSEUDO_LDS16rr:
      LdOpc = SPEX::LDS_R16;
      break;
    case SPEX::PSEUDO_LDS32rr:
      LdOpc = SPEX::LDS_R32;
      break;
    default:
      llvm_unreachable("Unexpected LDSrr pseudo");
    }

    BuildMI(MBB, I, DL, get(LdOpc)).addReg(Dst, RegState::Define);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_LDS8:
  case SPEX::PSEUDO_LDS16:
  case SPEX::PSEUDO_LDS32: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_LDS8:
      LdOpc = Off == 0 ? SPEX::LDS_R8 : SPEX::LDS_R8_I32;
      break;
    case SPEX::PSEUDO_LDS16:
      LdOpc = Off == 0 ? SPEX::LDS_R16 : SPEX::LDS_R16_I32;
      break;
    case SPEX::PSEUDO_LDS32:
      LdOpc = Off == 0 ? SPEX::LDS_R32 : SPEX::LDS_R32_I32;
      break;
    default:
      llvm_unreachable("unexpected LDS pseudo");
    }
    auto LdMI = BuildMI(MBB, I, DL, get(LdOpc), Dst);
    if (Off != 0)
      LdMI.addImm(Off);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ST8rr:
  case SPEX::PSEUDO_ST16rr:
  case SPEX::PSEUDO_ST32rr:
  case SPEX::PSEUDO_ST64rr: {
    Register Val = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX::ADD64_R)).addReg(Idx);

    unsigned StOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_ST8rr:
      StOpc = SPEX::ST8;
      break;
    case SPEX::PSEUDO_ST16rr:
      StOpc = SPEX::ST16;
      break;
    case SPEX::PSEUDO_ST32rr:
      StOpc = SPEX::ST32;
      break;
    case SPEX::PSEUDO_ST64rr:
      StOpc = SPEX::ST64;
      break;
    default:
      llvm_unreachable("Unexpected STrr pseudo");
    }

    BuildMI(MBB, I, DL, get(StOpc)).addReg(Val);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_ST8:
  case SPEX::PSEUDO_ST16:
  case SPEX::PSEUDO_ST32:
  case SPEX::PSEUDO_ST64: {
    Register Val = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX::MOVMOV64)).addReg(Base);
    unsigned StOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX::PSEUDO_ST8:
      StOpc = Off == 0 ? SPEX::ST_R8 : SPEX::ST_R8_I32;
      break;
    case SPEX::PSEUDO_ST16:
      StOpc = Off == 0 ? SPEX::ST_R16 : SPEX::ST_R16_I32;
      break;
    case SPEX::PSEUDO_ST32:
      StOpc = Off == 0 ? SPEX::ST_R32 : SPEX::ST_R32_I32;
      break;
    case SPEX::PSEUDO_ST64:
      StOpc = Off == 0 ? SPEX::ST_R64 : SPEX::ST_R64_I32;
      break;
    default:
      llvm_unreachable("unexpected ST pseudo");
    }
    auto StMI = BuildMI(MBB, I, DL, get(StOpc)).addReg(Val);
    if (Off != 0)
      StMI.addImm(Off);
    MI.eraseFromParent();
    return true;
  }
  case SPEX::PSEUDO_NOT32r: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();

    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV32), SPEX::RX).addReg(Src);
    BuildMI(MBB, MI, DL, get(SPEX::NOT32));
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV32_R), Dst);

    MI.eraseFromParent();
    return true;
  }

  case SPEX::PSEUDO_NOT64r: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();

    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64), SPEX::RX).addReg(Src);
    BuildMI(MBB, MI, DL, get(SPEX::NOT64));
    BuildMI(MBB, MI, DL, get(SPEX::MOVMOV64_R), Dst);

    MI.eraseFromParent();
    return true;
  }
  default:
    break;
  }

  return false;
}
