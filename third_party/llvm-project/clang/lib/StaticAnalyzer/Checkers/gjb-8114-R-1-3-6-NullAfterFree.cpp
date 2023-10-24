//=== misrac-2012-22_6-NullAfterFree.cpp - Check whether using the closed
// FILE
//-*-
// C++ -*-===//
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include <iostream>

using namespace clang;
using namespace clang::ento;

namespace {

class NullAfterFreeChecker
    : public Checker<check::PostCall, check::PreStmt<BinaryOperator>,
                     check::EndFunction> {
  mutable std::unique_ptr<BugType> BT;
  CallDescription FreeFunc;

  void reportBug(CheckerContext &C) const {
    if (!BT) {
      BT = std::make_unique<BugType>(
          this, "pointer variables must be set to NULL after being freed",
          categories::LogicError);
    }
    ExplodedNode *N = C.generateErrorNode();
    auto R = std::make_unique<PathSensitiveBugReport>(
        *BT, "pointer variables must be set to NULL after being freed", N);
    C.emitReport(std::move(R));
  }

public:
  NullAfterFreeChecker();
  // Process free(), add the var to the set.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  // Process assign NULL to after free ptr, remove the var from the map.
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
  // If the set is not empty, report error.
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &Ctx) const;
};

} // end anonymous namespace

REGISTER_SET_WITH_PROGRAMSTATE(StreamSet, const VarDecl *)

NullAfterFreeChecker::NullAfterFreeChecker()
    : FreeFunc({CDF_MaybeBuiltin, "free", 1}) {}

void NullAfterFreeChecker::checkPostCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (!FreeFunc.matches(Call))
    return;

  auto FreePtr = Call.getArgExpr(0);
  if (!FreePtr)
    return;
  const DeclRefExpr *DRE = cast<DeclRefExpr>(FreePtr->IgnoreParenImpCasts());
  const VarDecl *VD = cast<VarDecl>(DRE->getDecl());

  // Generate the next transition (an edge in the exploded graph).
  ProgramStateRef state = C.getState();

  state = state->add<StreamSet>(VD);
  C.addTransition(state);
}

void NullAfterFreeChecker::checkPreStmt(const BinaryOperator *B,
                                        CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  BinaryOperator::Opcode Op = B->getOpcode();
  if (Op != BO_Assign)
    return;

  // RHS must be NULL
  const auto RHS = B->getRHS();
  if (!RHS) {
    return;
  }
  if (!RHS->isNullPointerConstant(C.getASTContext(),
                                  Expr::NPC_ValueDependentIsNotNull)) {
    return;
  }
  const auto LHS = B->getLHS();
  if (!LHS) {
    return;
  }
  const DeclRefExpr *DRE = cast<DeclRefExpr>(LHS->IgnoreParenImpCasts());
  const VarDecl *VD = cast<VarDecl>(DRE->getDecl());

  if (state->contains<StreamSet>(VD)) {
    state = state->remove<StreamSet>(VD);
  }
  C.addTransition(state);
}

void NullAfterFreeChecker::checkEndFunction(const ReturnStmt *RS,
                                            CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  auto Entries = state->get<StreamSet>();
  if (!Entries.isEmpty()) {
    reportBug(C);
  }
}

void ento::registerNullAfterFreeChecker(CheckerManager &mgr) {
  auto *Checker = mgr.registerChecker<NullAfterFreeChecker>();
}

bool ento::shouldRegisterNullAfterFreeChecker(const CheckerManager &mgr) {
  return true;
}
