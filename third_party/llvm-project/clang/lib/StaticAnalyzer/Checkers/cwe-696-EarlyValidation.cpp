//===-- EarlyValidation.cpp --------------------------------*- C++ -*--//
//
// Implement CWE 696
// Idea behind this is:
// when we find that the return value of a function like realpath does not been
// validated,report error
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Checkers/NaiveCStdLibFunctionsInfo.h"
#include "clang/StaticAnalyzer/Checkers/Taint.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <iostream>

using namespace clang;
using namespace ento;
using namespace taint;
using namespace naive;

namespace {
class EarlyValidationChecker
    : public Checker<check::BranchCondition, check::PostCall,
                     check::EndFunction> {
  std::unique_ptr<BuiltinBug> EarlyValidationBugType;

private:
  static void addSymbol(SymbolRef symbolRef, CheckerContext &C);
  static ProgramStateRef removeSymbol(SymbolRef symbolRef, CheckerContext &C);

public:
  EarlyValidationChecker();
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
  void reportBug(CheckerContext &C, ExplodedNode *N,
                 std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;
  void checkAndRemoveSymbolInBoOrUo(const Expr *expr, CheckerContext &C) const;
};
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(UncheckedMap, SymbolRef, ExplodedNode *)

void EarlyValidationChecker::addSymbol(SymbolRef symbolRef, CheckerContext &C) {
  if (!symbolRef)
    return;

  ProgramStateRef State = C.getState();
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  // We should keep the exploded node where the value of the callee returns.
  State = State->set<UncheckedMap>(symbolRef, N);
  C.addTransition(State);
}

void EarlyValidationChecker::checkAndRemoveSymbolInBoOrUo(
    const Expr *expr, CheckerContext &C) const {
  // if the target we want is in a function call
  // i.e. if(strcmp("...", target, len) == 0) {}
  if (const auto *call_expr = dyn_cast<CallExpr>(expr)) {
    for (int i = 0; i < call_expr->getNumArgs(); ++i) {
      auto *arg = call_expr->getArg(i);
      SymbolRef arg_symbol = C.getSVal(arg).getAsSymbol();
      removeSymbol(arg_symbol, C);
    }
  } else if (SymbolRef sym = C.getSVal(expr).getAsSymbol()) {
    removeSymbol(sym, C);
  }
}

ProgramStateRef EarlyValidationChecker::removeSymbol(SymbolRef symbolRef,
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

EarlyValidationChecker::EarlyValidationChecker() {
  EarlyValidationBugType.reset(new BuiltinBug(this, "Early validation"));
}

void EarlyValidationChecker::checkPostCall(const CallEvent &Call,
                                           CheckerContext &C) const {
  if (!PathFunctions.contains(Call))
    return;

  QualType resultTy = Call.getResultType();

  SymbolRef resultVal = Call.getReturnValue().getAsSymbol();
  addSymbol(resultVal, C);
}
void EarlyValidationChecker::checkBranchCondition(const Stmt *Condition,
                                                  CheckerContext &C) const {
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
    checkAndRemoveSymbolInBoOrUo(uiOp->getSubExpr(), C);
  } else if (const BinaryOperator *biOp = dyn_cast<BinaryOperator>(Condition)) {
    /*
      int errorCode = func(int*)
      if (errorCode == 1) // value is checked
      if (errorcode != 2) // value is checked
    */
    BinaryOperator::Opcode Op = biOp->getOpcode();
    if (Op != BO_EQ && Op != BO_NE)
      return;

    checkAndRemoveSymbolInBoOrUo(biOp->getLHS(), C);
    checkAndRemoveSymbolInBoOrUo(biOp->getRHS(), C);
  }
}

void EarlyValidationChecker::checkEndFunction(const ReturnStmt *RS,
                                              CheckerContext &C) const {
  // When the function ends, we report every unchecked value.
  ProgramStateRef State = C.getState();
  UncheckedMapTy uncheckedMap = State->get<UncheckedMap>();
  for (auto &pr : uncheckedMap) {
    reportBug(C, pr.second);
  }
}

void EarlyValidationChecker::reportBug(
    CheckerContext &C, ExplodedNode *N,
    std::unique_ptr<BugReporterVisitor> Visitor) const {
  auto R = std::make_unique<PathSensitiveBugReport>(*EarlyValidationBugType,
                                                    "Early validation", N);
  R->addVisitor(std::move(Visitor));
  C.emitReport(std::move(R));
}

void ento::registerEarlyValidationChecker(CheckerManager &mgr) {
  mgr.registerChecker<EarlyValidationChecker>();
}

bool ento::shouldRegisterEarlyValidationChecker(const CheckerManager &mgr) {
  return true;
}
