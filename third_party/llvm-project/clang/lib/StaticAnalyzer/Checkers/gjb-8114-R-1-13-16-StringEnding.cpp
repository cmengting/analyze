//==- GJB-8114-R-1-13-16-StringEnding.cpp - string zero ending checker -*- C++
//-*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This file implements a string zero ending checker.
//===----------------------------------------------------------------------===//

#include "clang/AST/ParentMap.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/SourceManager.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "llvm/ADT/SmallSet.h"

using namespace clang;
using namespace ento;

namespace {
class StringEndingChecker
    : public Checker<check::Location, check::Bind, check::PreCall> {
public:
  unsigned int MaxLen = 0;
  void checkLocation(SVal L, bool IsLoad, const Stmt *S,
                     CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

private:
  mutable std::unique_ptr<BugType> BT;
  void checkBuffer(SVal Buffer, CheckerContext &C) const;
  bool checkZeroNotInCharBuffer(SVal Buffer, CheckerContext &C) const;

  void reportBug(CheckerContext &C) const {
    if (!BT) {
      BT = std::make_unique<BugType>(this, "string with no \\0 ending",
                                     categories::LogicError);
    }
    ExplodedNode *N = C.generateErrorNode();
    if (!N) {
      return;
    }
    auto R = std::make_unique<PathSensitiveBugReport>(
        *BT, "strings should end with \\0", N);
    C.emitReport(std::move(R));
  }
};
} // namespace

void StringEndingChecker::checkLocation(SVal L, bool IsLoad, const Stmt *S,
                                        CheckerContext &C) const {

  if (!IsLoad) {
    return;
  }
  checkBuffer(L, C);
}

void StringEndingChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                    CheckerContext &C) const {
  checkBuffer(V, C);
}

void StringEndingChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  unsigned int num = Call.getNumArgs();
  for (int i = 0; i < num; i++) {
    DefinedOrUnknownSVal Val =
        Call.getArgSVal(i).castAs<DefinedOrUnknownSVal>();
    checkBuffer(Val, C);
  }
}

void StringEndingChecker::checkBuffer(SVal Buffer, CheckerContext &C) const {
  DefinedOrUnknownSVal Location = Buffer.castAs<DefinedOrUnknownSVal>();
  if (!Location.getAs<Loc>()) {
    return;
  }
  if (checkZeroNotInCharBuffer(Location, C)) {
    reportBug(C);
  }
}

bool isInbound(DefinedOrUnknownSVal &Bound, DefinedOrUnknownSVal &Val,
               CheckerContext &C) {
  ProgramStateRef state = C.getState();
  ProgramStateRef StInBound = state->assumeInBound(Val, Bound, true);
  ProgramStateRef StOutBound = state->assumeInBound(Val, Bound, false);
  return StInBound && !StOutBound;
}

// char buffer zero checker, modified from misra 21.14
bool StringEndingChecker::checkZeroNotInCharBuffer(SVal Buffer,
                                                   CheckerContext &C) const {
  const MemRegion *R = Buffer.getAsRegion();
  if (!isa_and_nonnull<ElementRegion>(R)) {
    return false;
  }

  const ElementRegion *ER = dyn_cast<ElementRegion>(R);
  QualType Ty = ER->getValueType();
  if (!Ty->isAnyCharacterType()) {
    return false;
  }

  const MemRegion *SR = ER->getSuperRegion();
  if (!isa_and_nonnull<SubRegion>(SR)) {
    return false;
  }

  MemRegionManager &MRM = SR->getMemRegionManager();

  ASTContext &ACtx = C.getASTContext();
  SValBuilder &SVB = C.getSValBuilder();

  QualType CharTy = ACtx.CharTy;

  ProgramStateRef state = C.getState();

  DefinedOrUnknownSVal Size =
      getDynamicElementCount(state, SR, C.getSValBuilder(), Ty);
  auto EC = Size.getAs<DefinedOrUnknownSVal>();
  if (!EC) {
    return false;
  }

  NonLoc Idx = SVB.makeZeroArrayIndex();
  NonLoc Step = SVB.makeArrayIndex(1);
  QualType IdxTy = Idx.getType(ACtx);

  DefinedOrUnknownSVal MaxLenSVal =
      SVB.makeArrayIndex(MaxLen).castAs<DefinedOrUnknownSVal>();

  while (isInbound(*EC, Idx, C)) {
    if (MaxLen && !isInbound(MaxLenSVal, Idx, C)) {
      return false;
    }

    const ElementRegion *ER =
        MRM.getElementRegion(CharTy, Idx, dyn_cast<SubRegion>(SR), ACtx);
    SVal Val = state->getSVal(ER);

    if (!Val.isUnknownOrUndef()) {
      auto N = Val.getAs<NonLoc>();
      if (state->assume(*N, false) && !state->assume(*N, true)) {
        return false;
      }
    }
    auto newIdx =
        SVB.evalBinOp(state, BO_Add, Idx, Step, IdxTy).getAs<NonLoc>();
    if (!newIdx.hasValue()) {
      llvm_unreachable("Index must have value!");
    }
    Idx = newIdx.getValue();
  }
  return true;
}

void ento::registerStringEndingChecker(CheckerManager &mgr) {
  StringEndingChecker *C = mgr.registerChecker<StringEndingChecker>();
  C->MaxLen = mgr.getAnalyzerOptions().getCheckerIntegerOption(C, "MaxLen");
}

bool ento::shouldRegisterStringEndingChecker(const CheckerManager &mgr) {
  return true;
}
