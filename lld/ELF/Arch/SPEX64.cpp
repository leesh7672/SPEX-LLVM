//===- SPEX64.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"

using namespace llvm;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class SPEX64 final : public TargetInfo {
public:
  explicit SPEX64(Ctx &ctx) : TargetInfo(ctx) {}

  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
};
} // namespace

RelExpr SPEX64::getRelExpr(RelType type, const Symbol &s,
                           const uint8_t *loc) const {
  switch (type) {
  case 0:
    return R_NONE;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unknown relocation (" << type.v
             << ") against symbol " << &s;
    return R_NONE;
  }
}

void SPEX64::relocate(uint8_t *loc, const Relocation &rel,
                      uint64_t val) const {
  switch (rel.type) {
  case 0:
    return;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unrecognized relocation " << rel.type;
  }
}

void elf::setSPEX64TargetInfo(Ctx &ctx) { ctx.target.reset(new SPEX64(ctx)); }
