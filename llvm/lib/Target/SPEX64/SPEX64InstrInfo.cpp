//===-- SPEX64InstrInfo.cpp - SPEX64 instruction information --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include "llvm/Support/Debug.h"
//===----------------------------------------------------------------------===//

#include "SPEX64.h"
#include "SPEX64InstrInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SPEX64GenInstrInfo.inc"

using namespace llvm;

SPEX64InstrInfo::SPEX64InstrInfo(const TargetSubtargetInfo &STI)
    : SPEX64GenInstrInfo(STI, RI) {}

void SPEX64InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MI,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest, bool RenamableSrc) const {
  if (SPEX64::GPRRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(SPEX64::MOVMOV64))
        .addReg(SrcReg,
                getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
    BuildMI(MBB, MI, DL, get(SPEX64::MOVMOV64_R))
        .addReg(DestReg,
                RegState::Define | getRenamableRegState(RenamableDest));
    return;
  }

  if (DestReg == SPEX64::RX && SPEX64::GPRRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(SPEX64::MOVMOV64))
        .addReg(SrcReg,
                getKillRegState(KillSrc) | getRenamableRegState(RenamableSrc));
    return;
  }

  if (SPEX64::GPRRegClass.contains(DestReg) && SrcReg == SPEX64::RX) {
    BuildMI(MBB, MI, DL, get(SPEX64::MOVMOV64_R))
        .addReg(DestReg,
                RegState::Define | getRenamableRegState(RenamableDest));
    return;
  }

  report_fatal_error("SPEX64 copyPhysReg: unsupported register copy");
}

static unsigned getCondBranchOpcode(ISD::CondCode CC) {
  switch (CC) {
  case ISD::SETEQ:
    return SPEX64::BCC_eq_64;
  case ISD::SETNE:
    return SPEX64::BCC_ne_64;
  case ISD::SETLT:
    return SPEX64::BCC_lt_64;
  case ISD::SETLE:
    return SPEX64::BCC_le_64;
  case ISD::SETGT:
    return SPEX64::BCC_gt_64;
  case ISD::SETGE:
    return SPEX64::BCC_ge_64;
  case ISD::SETULT:
    return SPEX64::BCC_ltu_64;
  case ISD::SETULE:
    return SPEX64::BCC_leu_64;
  case ISD::SETUGT:
    return SPEX64::BCC_gtu_64;
  case ISD::SETUGE:
    return SPEX64::BCC_geu_64;
  default:
    report_fatal_error("SPEX64: unsupported branch condition");
  }
}

void SPEX64InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  (void)TRI;
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  // We only support integer GPR spills right now.
  if (RC != &SPEX64::GPRRegClass)
    report_fatal_error("SPEX64: unsupported spill register class");

  // FrameIndex will be resolved later by eliminateFrameIndex.
  // Use the existing pseudo store so the post-RA expander can materialize
  // base into RX and pick ST size.
  unsigned Opc = SPEX64::PSEUDO_ST64;
  BuildMI(MBB, MI, DL, get(Opc))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void SPEX64InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                           Register DestReg, int FrameIndex,
                                           const TargetRegisterClass *RC,
                                           Register VReg, unsigned SubReg,
                                           MachineInstr::MIFlag Flags) const {
  (void)TRI;
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  if (RC != &SPEX64::GPRRegClass)
    report_fatal_error("SPEX64: unsupported reload register class");

  unsigned Opc = SPEX64::PSEUDO_LDZ64;
  BuildMI(MBB, MI, DL, get(Opc), DestReg).addFrameIndex(FrameIndex).addImm(0);
}

bool SPEX64InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  MachineBasicBlock::iterator I = MI.getIterator();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  case SPEX64::ADJCALLSTACKDOWN:
  case SPEX64::ADJCALLSTACKUP: {
    // SPEX64 has no call stack to adjust in this ABI; erase if zero.
    // LLVM may still emit these pseudos around calls.
    int64_t Amt1 = MI.getOperand(0).isImm() ? MI.getOperand(0).getImm() : 0;
    int64_t Amt2 = MI.getOperand(1).isImm() ? MI.getOperand(1).getImm() : 0;
    if (Amt1 == 0 && Amt2 == 0) {
      MI.eraseFromParent();
      return true;
    }
    report_fatal_error(
        "SPEX64: ADJCALLSTACK* with non-zero amount is unsupported");
  }
  case SPEX64::CALL: {
    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(SPEX64::CALLI));
    for (const MachineOperand &MO : MI.operands())
      MIB.add(MO);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::CALLR: {
    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(SPEX64::CALL64));
    for (const MachineOperand &MO : MI.operands())
      MIB.add(MO);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LI8: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV8)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV8_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX64::LILI8_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV8_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LI16: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV16)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV16_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX64::LILI16_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV16_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LI32: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(SPEX64::LILI32_32)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LI64: {
    Register Dst = MI.getOperand(0).getReg();
    const MachineOperand &SrcOp = MI.getOperand(1);
    unsigned LiOpc = SPEX64::LILI64_64;
    if (SrcOp.isImm()) {
      int64_t Imm = SrcOp.getImm();
      if (isInt<32>(Imm))
        LiOpc = SPEX64::LILI64_32;
    } else if (SrcOp.isReg()) {
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(SrcOp.getReg());
      BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
      MI.eraseFromParent();
      return true;
    }
    BuildMI(MBB, I, DL, get(LiOpc)).add(SrcOp);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_ADD32rr:
  case SPEX64::PSEUDO_SUB32rr:
  case SPEX64::PSEUDO_AND32rr:
  case SPEX64::PSEUDO_OR32rr:
  case SPEX64::PSEUDO_XOR32rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    Register RHS = MI.getOperand(2).getReg();
    unsigned AluOpc = SPEX64::ADD32_R;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SUB32rr:
      AluOpc = SPEX64::SUB32_R;
      break;
    case SPEX64::PSEUDO_AND32rr:
      AluOpc = SPEX64::AND32_R;
      break;
    case SPEX64::PSEUDO_OR32rr:
      AluOpc = SPEX64::OR32_R;
      break;
    case SPEX64::PSEUDO_XOR32rr:
      AluOpc = SPEX64::XOR32_R;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addReg(RHS);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_ADD64rr:
  case SPEX64::PSEUDO_SUB64rr:
  case SPEX64::PSEUDO_AND64rr:
  case SPEX64::PSEUDO_OR64rr:
  case SPEX64::PSEUDO_XOR64rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    Register RHS = MI.getOperand(2).getReg();
    unsigned AluOpc = SPEX64::ADD64_R;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SUB64rr:
      AluOpc = SPEX64::SUB64_R;
      break;
    case SPEX64::PSEUDO_AND64rr:
      AluOpc = SPEX64::AND64_R;
      break;
    case SPEX64::PSEUDO_OR64rr:
      AluOpc = SPEX64::OR64_R;
      break;
    case SPEX64::PSEUDO_XOR64rr:
      AluOpc = SPEX64::XOR64_R;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addReg(RHS);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_ADD32ri:
  case SPEX64::PSEUDO_SUB32ri:
  case SPEX64::PSEUDO_AND32ri:
  case SPEX64::PSEUDO_OR32ri:
  case SPEX64::PSEUDO_XOR32ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned AluOpc = SPEX64::ADD32_I32;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SUB32ri:
      AluOpc = SPEX64::SUB32_I32;
      break;
    case SPEX64::PSEUDO_AND32ri:
      AluOpc = SPEX64::AND32_I32;
      break;
    case SPEX64::PSEUDO_OR32ri:
      AluOpc = SPEX64::OR32_I32;
      break;
    case SPEX64::PSEUDO_XOR32ri:
      AluOpc = SPEX64::XOR32_I32;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32)).addReg(Src);
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
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_ADD64ri:
  case SPEX64::PSEUDO_SUB64ri:
  case SPEX64::PSEUDO_AND64ri:
  case SPEX64::PSEUDO_OR64ri:
  case SPEX64::PSEUDO_XOR64ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    bool FitsI32 = isInt<32>(Imm);
    unsigned AluOpc = FitsI32 ? SPEX64::ADD64_I32 : SPEX64::ADD64_I64;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SUB64ri:
      AluOpc = FitsI32 ? SPEX64::SUB64_I32 : SPEX64::SUB64_I64;
      break;
    case SPEX64::PSEUDO_AND64ri:
      AluOpc = FitsI32 ? SPEX64::AND64_I32 : SPEX64::AND64_I64;
      break;
    case SPEX64::PSEUDO_OR64ri:
      AluOpc = FitsI32 ? SPEX64::OR64_I32 : SPEX64::OR64_I64;
      break;
    case SPEX64::PSEUDO_XOR64ri:
      AluOpc = FitsI32 ? SPEX64::XOR64_I32 : SPEX64::XOR64_I64;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(AluOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_SHL32ri:
  case SPEX64::PSEUDO_SRL32ri:
  case SPEX64::PSEUDO_SRA32ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned ShOpc = SPEX64::SHL32;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SRL32ri:
      ShOpc = SPEX64::SHR32;
      break;
    case SPEX64::PSEUDO_SRA32ri:
      ShOpc = SPEX64::SAR32;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32)).addReg(Src);
    BuildMI(MBB, I, DL, get(ShOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV32_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_SHL64ri:
  case SPEX64::PSEUDO_SRL64ri:
  case SPEX64::PSEUDO_SRA64ri: {
    Register Dst = MI.getOperand(0).getReg();
    Register Src = MI.getOperand(1).getReg();
    int64_t Imm = MI.getOperand(2).getImm();
    unsigned ShOpc = SPEX64::SHL64;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_SRL64ri:
      ShOpc = SPEX64::SHR64;
      break;
    case SPEX64::PSEUDO_SRA64ri:
      ShOpc = SPEX64::SAR64;
      break;
    default:
      break;
    }
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Src);
    BuildMI(MBB, I, DL, get(ShOpc)).addImm(Imm);
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64_R), Dst);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LDZ8rr:
  case SPEX64::PSEUDO_LDZ16rr:
  case SPEX64::PSEUDO_LDZ32rr:
  case SPEX64::PSEUDO_LDZ64rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX64::ADD64_R)).addReg(Idx);

    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_LDZ8rr:
      LdOpc = SPEX64::LDZ_R8;
      break;
    case SPEX64::PSEUDO_LDZ16rr:
      LdOpc = SPEX64::LDZ_R16;
      break;
    case SPEX64::PSEUDO_LDZ32rr:
      LdOpc = SPEX64::LDZ_R32;
      break;
    case SPEX64::PSEUDO_LDZ64rr:
      LdOpc = SPEX64::LDZ_R64;
      break;
    default:
      llvm_unreachable("Unexpected LDZrr pseudo");
    }

    BuildMI(MBB, I, DL, get(LdOpc)).addReg(Dst, RegState::Define);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LDZ8:
  case SPEX64::PSEUDO_LDZ16:
  case SPEX64::PSEUDO_LDZ32:
  case SPEX64::PSEUDO_LDZ64: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_LDZ8:
      LdOpc = Off == 0 ? SPEX64::LDZ_R8 : SPEX64::LDZ_R8_I32;
      break;
    case SPEX64::PSEUDO_LDZ16:
      LdOpc = Off == 0 ? SPEX64::LDZ_R16 : SPEX64::LDZ_R16_I32;
      break;
    case SPEX64::PSEUDO_LDZ32:
      LdOpc = Off == 0 ? SPEX64::LDZ_R32 : SPEX64::LDZ_R32_I32;
      break;
    case SPEX64::PSEUDO_LDZ64:
      LdOpc = Off == 0 ? SPEX64::LDZ_R64 : SPEX64::LDZ_R64_I32;
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
  case SPEX64::PSEUDO_LDS8rr:
  case SPEX64::PSEUDO_LDS16rr:
  case SPEX64::PSEUDO_LDS32rr: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX64::ADD64_R)).addReg(Idx);

    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_LDS8rr:
      LdOpc = SPEX64::LDS_R8;
      break;
    case SPEX64::PSEUDO_LDS16rr:
      LdOpc = SPEX64::LDS_R16;
      break;
    case SPEX64::PSEUDO_LDS32rr:
      LdOpc = SPEX64::LDS_R32;
      break;
    default:
      llvm_unreachable("Unexpected LDSrr pseudo");
    }

    BuildMI(MBB, I, DL, get(LdOpc)).addReg(Dst, RegState::Define);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_LDS8:
  case SPEX64::PSEUDO_LDS16:
  case SPEX64::PSEUDO_LDS32: {
    Register Dst = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    unsigned LdOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_LDS8:
      LdOpc = Off == 0 ? SPEX64::LDS_R8 : SPEX64::LDS_R8_I32;
      break;
    case SPEX64::PSEUDO_LDS16:
      LdOpc = Off == 0 ? SPEX64::LDS_R16 : SPEX64::LDS_R16_I32;
      break;
    case SPEX64::PSEUDO_LDS32:
      LdOpc = Off == 0 ? SPEX64::LDS_R32 : SPEX64::LDS_R32_I32;
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
  case SPEX64::PSEUDO_ST8rr:
  case SPEX64::PSEUDO_ST16rr:
  case SPEX64::PSEUDO_ST32rr:
  case SPEX64::PSEUDO_ST64rr: {
    Register Val = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    Register Idx = MI.getOperand(2).getReg();

    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    BuildMI(MBB, I, DL, get(SPEX64::ADD64_R)).addReg(Idx);

    unsigned StOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_ST8rr:
      StOpc = SPEX64::ST8;
      break;
    case SPEX64::PSEUDO_ST16rr:
      StOpc = SPEX64::ST16;
      break;
    case SPEX64::PSEUDO_ST32rr:
      StOpc = SPEX64::ST32;
      break;
    case SPEX64::PSEUDO_ST64rr:
      StOpc = SPEX64::ST64;
      break;
    default:
      llvm_unreachable("Unexpected STrr pseudo");
    }

    BuildMI(MBB, I, DL, get(StOpc)).addReg(Val);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_ST8:
  case SPEX64::PSEUDO_ST16:
  case SPEX64::PSEUDO_ST32:
  case SPEX64::PSEUDO_ST64: {
    Register Val = MI.getOperand(0).getReg();
    Register Base = MI.getOperand(1).getReg();
    int64_t Off = MI.getOperand(2).getImm();
    BuildMI(MBB, I, DL, get(SPEX64::MOVMOV64)).addReg(Base);
    unsigned StOpc = 0;
    switch (MI.getOpcode()) {
    case SPEX64::PSEUDO_ST8:
      StOpc = Off == 0 ? SPEX64::ST_R8 : SPEX64::ST_R8_I32;
      break;
    case SPEX64::PSEUDO_ST16:
      StOpc = Off == 0 ? SPEX64::ST_R16 : SPEX64::ST_R16_I32;
      break;
    case SPEX64::PSEUDO_ST32:
      StOpc = Off == 0 ? SPEX64::ST_R32 : SPEX64::ST_R32_I32;
      break;
    case SPEX64::PSEUDO_ST64:
      StOpc = Off == 0 ? SPEX64::ST_R64 : SPEX64::ST_R64_I32;
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
  case SPEX64::PSEUDO_BR: {
    MachineBasicBlock *Dest = MI.getOperand(0).getMBB();
    BuildMI(MBB, I, DL, get(SPEX64::JMP64)).addMBB(Dest);
    MI.eraseFromParent();
    return true;
  }
  case SPEX64::PSEUDO_BR_CC32:
  case SPEX64::PSEUDO_BR_CC64: {
    Register LHS = MI.getOperand(0).getReg();
    Register RHS = MI.getOperand(1).getReg();
    MachineBasicBlock *Dest = MI.getOperand(2).getMBB();
    auto CC = static_cast<ISD::CondCode>(MI.getOperand(3).getImm());
    bool Is32 = MI.getOpcode() == SPEX64::PSEUDO_BR_CC32;
    unsigned CmpOpc = Is32 ? SPEX64::CMP32_R : SPEX64::CMP64_R;
    unsigned MovOpc = Is32 ? SPEX64::MOVMOV32 : SPEX64::MOVMOV64;
    BuildMI(MBB, I, DL, get(MovOpc)).addReg(LHS);
    BuildMI(MBB, I, DL, get(CmpOpc)).addReg(RHS);
    BuildMI(MBB, I, DL, get(getCondBranchOpcode(CC))).addMBB(Dest);
    MI.eraseFromParent();
    return true;
  }
  default:
    break;
  }

  return false;
}
