//===-- UncheckedReturnPointer.cpp --------------------------------*- C++ -*--//
//
// Implement CWE 690: Unchecked Return Value to NULL Pointer Dereference.
//
// This defines UncheckedReturnPointerChecker, a checker that performs checks for
// the return pointer that should be checked to avoid null pointer dereference.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Checkers/Taint.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;
using namespace taint;

namespace {
class UncheckedReturnPointerChecker
    : public Checker<check::BranchCondition, check::Bind,
                     check::EndFunction> {
  std::unique_ptr<BuiltinBug> UncheckedReturnValueBugType;

private:
  static void addSymbol(SymbolRef symbolRef, CheckerContext &C);
  static ProgramStateRef removeSymbol(SymbolRef symbolRef, CheckerContext &C);
  static bool needReport(const VarDecl *vd, const CallExpr *call);

public:
  UncheckedReturnPointerChecker();
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &) const;
  void reportBug(CheckerContext &C, ExplodedNode *N,
                 std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;
};
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(UncheckedMap, SymbolRef, ExplodedNode *)

void UncheckedReturnPointerChecker::addSymbol(SymbolRef symbolRef,
                                            CheckerContext &C) {
  if (!symbolRef)
    return;

  ProgramStateRef State = C.getState();
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  // We should keep the exploded node where the value of the callee returns.
  State = State->set<UncheckedMap>(symbolRef, N);
  C.addTransition(State);
}

ProgramStateRef UncheckedReturnPointerChecker::removeSymbol(SymbolRef symbolRef,
                                                          CheckerContext &C) {
  //  When we use if-stmt, while-stmt, and so on to check the value and delete
  //  it from the state.
  ProgramStateRef State = C.getState();
  if (State->contains<UncheckedMap>(symbolRef)) {
    State = State->remove<UncheckedMap>(symbolRef);
    C.addTransition(State);
  }
  return State;
}

UncheckedReturnPointerChecker::UncheckedReturnPointerChecker() {
  UncheckedReturnValueBugType.reset(
      new BuiltinBug(this, "Unchecked Return Value to NULL Pointer Dereference"));
}

bool UncheckedReturnPointerChecker::needReport(const VarDecl *vd, const CallExpr *call) {
  // if the return value is never used, don't report
  if (!vd->isUsed())
    return false;
  // if the return value is not a pointer type, don't report
  auto *callee = call->getDirectCallee();
  if (!callee) 
    return false;
  if (!callee->getReturnType()->isAnyPointerType())
    return false;
  return true;
}

void UncheckedReturnPointerChecker::checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &C) const {
    // We will match Bind like
    // int *a = func(); (VarDecl)
    // int *a; a = func(); (BinaryOperator)
    if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(S)) {
        if (BO->getOpcode() != clang::BO_Assign)
            return;
        Expr *lhs = BO->getLHS()->IgnoreImpCasts();
        Expr *rhs = BO->getRHS()->IgnoreImpCasts();
        if (!isa<DeclRefExpr>(lhs))
            return;
        DeclRefExpr* declref = dyn_cast<DeclRefExpr>(lhs);
        if (!isa<VarDecl>(declref->getDecl()))
            return;
        VarDecl* vd = dyn_cast<VarDecl>(declref->getDecl());
        if (!isa<CallExpr>(rhs))
            return;
        CallExpr* call = dyn_cast<CallExpr>(rhs);

        if (needReport(vd, call))
          addSymbol(Val.getAsSymbol(), C);
    }
    else if (const DeclStmt * DS = dyn_cast<DeclStmt>(S)) {
      for (auto *decl: DS->getDeclGroup()) {
        if (!isa<VarDecl>(decl))
          return;
        VarDecl* vd = dyn_cast<VarDecl>(decl);
        const Expr *init = vd->getAnyInitializer()->IgnoreImpCasts();
        if (!init || !isa<CallExpr>(init))
          return;
        const CallExpr* call = dyn_cast<CallExpr>(init);

        if (needReport(vd, call))
          addSymbol(Val.getAsSymbol(), C);
      }
    }
    return;
}

void UncheckedReturnPointerChecker::checkBranchCondition(
    const Stmt *Condition, CheckerContext &C) const {
  if (SymbolRef symbolRef = C.getSVal(Condition).getAsSymbol()) {
    /*
      auto addr = malloc(...)
      if (addr) // value is checked
    */
    removeSymbol(symbolRef, C);
  } else if (const UnaryOperator *uiOp = dyn_cast<UnaryOperator>(Condition)) {
    /*
      auto addr = malloc(...)
      if (!addr) // value is checked
    */
    UnaryOperator::Opcode Op = uiOp->getOpcode();
    if (Op != UO_LNot)
      return;
    SymbolRef subExpr = C.getSVal(uiOp->getSubExpr()).getAsSymbol();
    if (subExpr)
      removeSymbol(subExpr, C);
  } else if (const BinaryOperator *biOp = dyn_cast<BinaryOperator>(Condition)) {
    /*
      int errorCode = func(int*)
      if (errorCode == 1) // value is checked
      if (errorcode != 2) // value is checked
    */
    BinaryOperator::Opcode Op = biOp->getOpcode();
    if (Op != BO_EQ && Op != BO_NE)
      return;
    SymbolRef lhs = C.getSVal(biOp->getLHS()).getAsSymbol();
    if (lhs)
      removeSymbol(lhs, C);
    SymbolRef rhs = C.getSVal(biOp->getRHS()).getAsSymbol();
    if (rhs)
      removeSymbol(rhs, C);
  }
}

void UncheckedReturnPointerChecker::checkEndFunction(const ReturnStmt *RS,
                                                   CheckerContext &C) const {
  // When the function ends, we report every unchecked value.
  ProgramStateRef State = C.getState();
  UncheckedMapTy uncheckedMap = State->get<UncheckedMap>();
  for (auto &pr : uncheckedMap) {
    reportBug(C, pr.second);
  }
}

void UncheckedReturnPointerChecker::reportBug(
    CheckerContext &C, ExplodedNode *N,
    std::unique_ptr<BugReporterVisitor> Visitor) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      *UncheckedReturnValueBugType, "Unchecked Return Value to NULL Pointer Dereference", N);
  R->addVisitor(std::move(Visitor));
  C.emitReport(std::move(R));
}

void ento::registerUncheckedReturnPointerChecker(CheckerManager &mgr) {
  mgr.registerChecker<UncheckedReturnPointerChecker>();
}

bool ento::shouldRegisterUncheckedReturnPointerChecker(
    const CheckerManager &mgr) {
  return true;
}
