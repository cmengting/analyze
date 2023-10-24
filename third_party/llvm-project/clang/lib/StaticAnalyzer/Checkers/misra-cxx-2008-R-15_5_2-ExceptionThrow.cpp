//=== misra-cxx-2008-R-15_5_2-ExceptionThrow.cpp ----*- C++ -*--===//

#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class ExceptionThrowChecker
    : public Checker<check::BeginFunction, check::PreStmt<CXXThrowExpr>,
                     check::EndFunction> {

  void ReportError(SourceLocation Loc, CheckerContext &C) const;

  mutable std::unique_ptr<BuiltinBug> BT_ExceptionThrow;

public:
  void checkBeginFunction(CheckerContext &C) const;
  void checkPreStmt(const CXXThrowExpr *E, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
};
} // end namespace

struct Excepts {
public:
  std::set<const QualType *> excepts;

  bool operator==(const Excepts &E2) const { return excepts == E2.excepts; }
  void Profile(llvm::FoldingSetNodeID &ID) const {
    for (auto it = excepts.begin(); it != excepts.end(); it++) {
      ID.AddString((*it)->getAsString());
    }
  }
};

REGISTER_MAP_WITH_PROGRAMSTATE(FuncExcepts, const FunctionDecl *, Excepts)
REGISTER_SET_WITH_PROGRAMSTATE(
    CheckingFuncs,
    const FunctionDecl *) // Set will ignore duplicating function calls,
                          // may introduce negative results.

void ExceptionThrowChecker::ReportError(SourceLocation Loc,
                                        CheckerContext &C) const {
  if (Loc.isInvalid())
    return;
  if (!BT_ExceptionThrow)
    BT_ExceptionThrow.reset(
        new BuiltinBug(this, "[misracxx-2008-15.5.2]",
                       "violation of misra_cxx_2008: rule_15_5_2"));
  PathDiagnosticLocation Pos(Loc, C.getSourceManager());
  auto R = std::make_unique<BasicBugReport>(
      *BT_ExceptionThrow, BT_ExceptionThrow->getDescription(), Pos);
  C.emitReport(std::move(R));
}

void ExceptionThrowChecker::checkBeginFunction(CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  const FunctionDecl *FD =
      dyn_cast<FunctionDecl>(C.getLocationContext()->getDecl());
  if (!FD)
    return;
  if (State->contains<CheckingFuncs>(FD))
    return;
  State = C.getState()->add<CheckingFuncs>(FD);
  if (State->get<FuncExcepts>(FD) != NULL) {
    C.addTransition(State);
    return; // exception spec is already recorded
  }
  auto ProtoType = FD->getType()->getAs<FunctionProtoType>();
  Excepts E;
  if (ProtoType->getExceptionSpecType() == clang::EST_Dynamic) {
    for (auto it = ProtoType->exceptions().begin();
         it != ProtoType->exceptions().end(); it++) {
      E.excepts.insert(it);
    }
  }
  State = State->set<FuncExcepts>(FD, E);
  C.addTransition(State);
}

void ExceptionThrowChecker::checkPreStmt(const CXXThrowExpr *E,
                                         CheckerContext &C) const {
  const QualType EType = E->getSubExpr()->getType();
  CheckingFuncsTy Funcs = C.getState()->get<CheckingFuncs>();
  for (auto Func : Funcs) {
    bool ok = false;
    const Excepts *Exs = C.getState()->get<FuncExcepts>(Func);
    if (Exs == NULL) {
      continue;
    }
    if (Exs->excepts.empty()) {
      continue;
    }
    for (auto t : Exs->excepts) {
      if (*t == EType) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      ReportError(Func->getBeginLoc(), C);
    }
  }
}

void ExceptionThrowChecker::checkEndFunction(const ReturnStmt *RS,
                                             CheckerContext &C) const {
  const FunctionDecl *FD = dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl());
  if (!FD)
    return;
  if (C.getState()->contains<CheckingFuncs>(FD))
    C.addTransition(C.getState()->remove<CheckingFuncs>(FD));
}

void ento::registerExceptionThrowChecker(CheckerManager &mgr) {
  mgr.registerChecker<ExceptionThrowChecker>();
}

bool ento::shouldRegisterExceptionThrowChecker(const CheckerManager &mgr) {
  return true;
}
