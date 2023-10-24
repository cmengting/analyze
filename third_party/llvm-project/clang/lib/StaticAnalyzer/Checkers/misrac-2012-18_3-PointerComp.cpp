//=== misrac-2012-18_3 PointerCompChecker.cpp - Pointer comparisons checker ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This files defines PointerCompChecker: it's modified on top of the original
// clang static checker PointerSubChecker.cpp
//
// rule 18.3:
// The relational operators >, >=, <, <= shall not be applied to objects of
// pointer type except where they point into the same object
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class PointerCompChecker
  : public Checker< check::PreStmt<BinaryOperator> > {
  mutable std::unique_ptr<BuiltinBug> BT;
  void reportPointerCompMisuse(const BinaryOperator *B, CheckerContext &C) const;

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};

void PointerCompChecker::reportPointerCompMisuse(const BinaryOperator *B,
                                                    CheckerContext &C) const {
    if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
      if (!BT)
        BT.reset(
            new BuiltinBug(this, "[misrac-2012-18.3] ",
                           "Pointer compare violation of misra_c_2012: rule_18_3"));
      auto R =
          std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);
      R->addRange(B->getSourceRange());
      C.emitReport(std::move(R));
    }
  }
}

void PointerCompChecker::checkPreStmt(const BinaryOperator *B,
                                     CheckerContext &C) const {
  // When doing pointer comparisons, if the two pointers do not point to the
  // same memory chunk, emit a warning.
  if (B->getOpcode() != clang::BO_GT &&
      B->getOpcode() != clang::BO_GE &&
      B->getOpcode() != clang::BO_LE &&
      B->getOpcode() != clang::BO_LT)
    return;

  // Both LHS and RHS must be PointerType
  if (!(B->getLHS()->getType()->isPointerType() &&
      B->getRHS()->getType()->isPointerType()))
    return;

  // check type
  if (B->getLHS()->getType().getCanonicalType() !=
      B->getRHS()->getType().getCanonicalType()) {
        reportPointerCompMisuse(B, C);
        return;
      }


  SVal LV = C.getSVal(B->getLHS());
  SVal RV = C.getSVal(B->getRHS());

  const MemRegion *LR = LV.getAsRegion();
  const MemRegion *RR = RV.getAsRegion();

  // LR or RR is not a region (eg. NULL)
  if (!(LR && RR)){
    reportPointerCompMisuse(B, C);
    return;
  }

  const MemRegion *BaseLR = LR->getBaseRegion();
  const MemRegion *BaseRR = RR->getBaseRegion();

  if (BaseLR == BaseRR)
    return;

  // When doing pointer Comparisons, if the two pointers do not point to the
  // same memory chunk, emit a warning.
  reportPointerCompMisuse(B, C);
}

void ento::registerPointerCompareChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerCompChecker>();
}

bool ento::shouldRegisterPointerCompareChecker(const CheckerManager &mgr) {
  return true;
}
