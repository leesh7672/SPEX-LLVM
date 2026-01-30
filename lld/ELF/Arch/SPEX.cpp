//===- SPEX.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Symbols.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class SPEX final : public TargetInfo {
public:
  explicit SPEX(Ctx &ctx) : TargetInfo(ctx) {}

  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
};
} // namespace

RelExpr SPEX::getRelExpr(RelType type, const Symbol &s,
                           const uint8_t *loc) const {
  switch (type) {
  case R_SPEX_NONE:
    return R_NONE;
  case R_SPEX_32:
  case R_SPEX_64:
    return R_ABS;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unknown relocation (" << type.v
             << ") against symbol " << &s;
    return R_NONE;
  }
}

void SPEX::relocate(uint8_t *loc, const Relocation &rel,
                      uint64_t val) const {
  switch (rel.type) {
  case R_SPEX_NONE:
    return;
  case R_SPEX_32:
    llvm::support::endian::write32le(loc, static_cast<uint32_t>(val));
    return;
  case R_SPEX_64:
    llvm::support::endian::write64le(loc, val);
    return;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unrecognized relocation " << rel.type;
  }
}

void elf::setSPEXTargetInfo(Ctx &ctx) { ctx.target.reset(new SPEX(ctx)); }
