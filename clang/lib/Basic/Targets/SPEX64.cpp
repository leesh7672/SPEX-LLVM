//===--- SPEX64.cpp - Implement SPEX64 target feature support -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SPEX64.h"
#include "Targets.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace targets {

void SPEX64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  // Generic
  Builder.defineMacro("__ELF__");

  // Target macros
  Builder.defineMacro("__spex64__");
  Builder.defineMacro("__SPEX64__");
  Builder.defineMacro("__SPEX64", "1");

  // Data model (64-bit)
  Builder.defineMacro("__LP64__");
  Builder.defineMacro("_LP64");
}

SPEX64TargetInfo::SPEX64TargetInfo(const llvm::Triple &Triple,
                                   const TargetOptions &Opts)
    : TargetInfo(Triple) {
  // CPU/ABI: keep minimal and strict first.
  // If you later introduce variants, hook them here.
  resetDataLayout(
      // Little-endian, ELF mangling, 64-bit pointers.
      // You can refine (stack alignment, native widths) later.
      "e-m:e-p:64:64-i64:64-n64-S128");

  // Fundamental sizes
  PointerWidth = 64;
  PointerAlign = 64;

  LongWidth = 64;
  LongAlign = 64;

  SizeType = UnsignedLong;
  PtrDiffType = SignedLong;
  IntPtrType = SignedLong;

  // Default ABI choices
  // For a bare-metal/unknown environment, this is usually acceptable.
  // If you add an actual ABI later, you can switch accordingly.
  TLSSupported = false;

  // IEEE float defaults (if your ISA/ABI differs, tweak)
  FloatWidth = 32;
  FloatAlign = 32;
  DoubleWidth = 64;
  DoubleAlign = 64;
  LongDoubleWidth = 64;
  LongDoubleAlign = 64;
  LongDoubleFormat = &llvm::APFloat::IEEEdouble();

  // Builtin types
  WCharType = UnsignedInt; // You can change if you want signed.
  WIntType = UnsignedInt;

  // Use generic va_list model for now.
  // If you later implement a proper ABI, update this.
  // Many bare-metal targets use VoidPtrBuiltinVaList.
  // (Clang will typedef __builtin_va_list appropriately.)
  // See other targets for patterns.
}

TargetInfo::BuiltinVaListKind SPEX64TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

llvm::SmallVector<Builtin::InfosShard>
SPEX64TargetInfo::getTargetBuiltins() const {
  return {};
}

std::string_view SPEX64TargetInfo::getClobbers() const { return ""; }

ArrayRef<const char *> SPEX64TargetInfo::getGCCRegNames() const { return {}; }

bool SPEX64TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  return true;
}

ArrayRef<TargetInfo::GCCRegAlias>
SPEX64TargetInfo::getGCCRegAliases() const {
  return {};
}

} // namespace targets
} // namespace clang
