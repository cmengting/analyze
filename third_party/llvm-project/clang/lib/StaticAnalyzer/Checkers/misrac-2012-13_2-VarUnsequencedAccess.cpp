//=== VarUnsequencedAccess.cpp - volatile variable unsequenced access checker
//------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// rule 13.2:
// To keep the value of an expression and its PSE same under all permitted
// evaluation orders I want to make sure that no volatile qulified variable
// exists on both sides of a binary operator.
//
// function argument should not be volatile qualified
//===----------------------------------------------------------------------===//
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"

using namespace clang;
using namespace clang::ento;

namespace {
class VarUnsequencedAccessChecker
    : public Checker<check::PreStmt<BinaryOperator>, check::PreCall> {
public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

private:
  mutable std::unique_ptr<BugType> BT;

  void reportBug(CheckerContext &C) const {
    if (!BT) {
      BT = std::make_unique<BugType>(
          this, "misra-c2012-13.2: ", categories::LogicError);
    }
    ExplodedNode *N = C.generateNonFatalErrorNode();
    auto R = std::make_unique<PathSensitiveBugReport>(
        *BT,
        "volatile type can only be accessed at most once between two sequence "
        "points.",
        N);
    C.emitReport(std::move(R));
  }
};

bool isVolatileAccess(QualType QT) {
  if (QT.isVolatileQualified()) {
    return true;
  }
  while (QT->isPointerType()) {
    QT = QT->getPointeeType();
    if (QT.isVolatileQualified()) {
      return true;
    }
  }
  return false;
}
} // namespace

void VarUnsequencedAccessChecker::checkPreStmt(const BinaryOperator *B,
                                               CheckerContext &C) const {
  if (B->isAssignmentOp()) {
    return;
  }

  if (B->getLHS() == nullptr || B->getRHS() == nullptr) {
      return;
  }

  QualType LHS = B->getLHS()->IgnoreCasts()->getType();
  QualType RHS = B->getRHS()->IgnoreCasts()->getType();
  // TODO: handle if LHS and RHS are invalid Expr
  if (isVolatileAccess(LHS) && isVolatileAccess(RHS)) {
    reportBug(C);
  }
}

void VarUnsequencedAccessChecker::checkPreCall(const CallEvent &Call,
                                               CheckerContext &C) const {
  if (Call.getNumArgs() < 2) {
    return;
  }

  unsigned counter = 0;
  for (unsigned i = 0; i < Call.getNumArgs(); ++i) {
    if (Call.getArgExpr(i) == nullptr) {
      continue;
    }
    QualType QT = Call.getArgExpr(i)->IgnoreCasts()->getType();
    if (isVolatileAccess(QT)) {
      counter++;
    }
    if (counter > 1) {
      reportBug(C);
      return;
    }
  }
}

void ento::registerVarUnsequencedAccessChecker(CheckerManager &mgr) {
  mgr.registerChecker<VarUnsequencedAccessChecker>();
}

bool ento::shouldRegisterVarUnsequencedAccessChecker(
    const CheckerManager &mgr) {
  return true;
}
