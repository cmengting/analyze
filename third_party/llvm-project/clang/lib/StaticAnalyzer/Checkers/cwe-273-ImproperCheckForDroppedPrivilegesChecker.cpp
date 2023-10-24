//== ImproperCheckForDroppedPrivilegesChecker.cpp - Improper Check for Dropped
// Privileges -----*- C++ -*--==//
//
// This defines ImproperCheckForDroppedPrivilegesChecker, a checker that
// performs checks for the return value from setuid, seteuid, setgid, setegid
// that should be checked.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class ImproperCheckForDroppedPrivilegesChecker
    : public Checker<check::PostCall, check::BranchCondition,
                     check::EndFunction> {
  std::unique_ptr<BuiltinBug> ImproperCheckForDroppedPrivilegesBugType;
  // check-list:
  const CallDescriptionSet ReturnValueNeedCheckFunctions = {
      {CDF_MaybeBuiltin, "setuid", 1},
      {CDF_MaybeBuiltin, "seteuid", 1},
      {CDF_MaybeBuiltin, "setgid", 1},
      {CDF_MaybeBuiltin, "setegid", 1},
  };

private:
  static void addSymbol(SymbolRef symbolRef, CheckerContext &C);
  static ProgramStateRef removeSymbol(std::vector<SymbolRef> symbolRefVec,
                                      CheckerContext &C);

public:
  ImproperCheckForDroppedPrivilegesChecker();
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
  void reportBug(CheckerContext &C, ExplodedNode *N,
                 std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;
};
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(ImproperMap, SymbolRef, ExplodedNode *)

void ImproperCheckForDroppedPrivilegesChecker::addSymbol(SymbolRef symbolRef,
                                                         CheckerContext &C) {
  if (!symbolRef)
    return;

  ProgramStateRef State = C.getState();
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  // We should keep the exploded node where the value of the callee returns.
  State = State->set<ImproperMap>(symbolRef, N);
  C.addTransition(State);
}

ProgramStateRef ImproperCheckForDroppedPrivilegesChecker::removeSymbol(
    std::vector<SymbolRef> symbolRefVec, CheckerContext &C) {
  //  When we use if-stmt, while-stmt, and so on to check the value and delete
  //  it from the state.
  ProgramStateRef State = C.getState();
  for (auto it = symbolRefVec.begin(); it != symbolRefVec.end(); ++it) {
    SymbolRef symbolRef = *it;
    if (State->contains<ImproperMap>(symbolRef)) {
      State = State->remove<ImproperMap>(symbolRef);
    }
  }
  C.addTransition(State);
  return State;
}

ImproperCheckForDroppedPrivilegesChecker::
    ImproperCheckForDroppedPrivilegesChecker() {
  ImproperCheckForDroppedPrivilegesBugType.reset(
      new BuiltinBug(this, "Improper Check for Dropped Privileges"));
}

void ImproperCheckForDroppedPrivilegesChecker::checkPostCall(
    const CallEvent &Call, CheckerContext &C) const {
  QualType resultTy = Call.getResultType();
  if (resultTy->isVoidType())
    return;

  if (!ReturnValueNeedCheckFunctions.contains(Call))
    return;

  SymbolRef resultVal = Call.getReturnValue().getAsSymbol();
  addSymbol(resultVal, C);
}

void ImproperCheckForDroppedPrivilegesChecker::checkBranchCondition(
    const Stmt *Condition, CheckerContext &C) const {
  if (SymbolRef symbolRef = C.getSVal(Condition).getAsSymbol()) {
    // auto addr = malloc(...)
    // if (addr) // value is checked
    removeSymbol({symbolRef}, C);
  } else if (const UnaryOperator *uiOp = dyn_cast<UnaryOperator>(Condition)) {
    // auto addr = malloc(...)
    // if (!addr) // value is checked
    UnaryOperator::Opcode Op = uiOp->getOpcode();
    if (Op != UO_LNot)
      return;
    SymbolRef subExpr = C.getSVal(uiOp->getSubExpr()).getAsSymbol();
    if (subExpr)
      removeSymbol({subExpr}, C);
  } else if (const BinaryOperator *biOp = dyn_cast<BinaryOperator>(Condition)) {
    // int errorCode = func(int*)
    // if (errorCode == 1) // value is checked
    // if (errorcode != 2) // value is checked
    BinaryOperator::Opcode Op = biOp->getOpcode();
    if (!(Op == BO_LT || Op == BO_GT || Op == BO_LE || Op == BO_GE ||
          Op == BO_EQ || Op == BO_NE))
      return;
    auto symbolRefVec = [&]() -> std::vector<SymbolRef> {
      std::vector<SymbolRef> symbolRefVec;
      SymbolRef lhs = C.getSVal(biOp->getLHS()).getAsSymbol();
      if (lhs)
        symbolRefVec.push_back(lhs);
      SymbolRef rhs = C.getSVal(biOp->getRHS()).getAsSymbol();
      if (rhs)
        symbolRefVec.push_back(rhs);
      return symbolRefVec;
    }();
    removeSymbol(symbolRefVec, C);
  }
}

void ImproperCheckForDroppedPrivilegesChecker::checkEndFunction(
    const ReturnStmt *RS, CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (RS) {
    // void* mallocWrapper(...) {
    //   ...
    //   return malloc(...); // The return value can be checked later.
    // }
    if (const Expr *retValue = RS->getRetValue()) {
      if (SymbolRef symbolRef = C.getSVal(retValue).getAsSymbol())
        State = removeSymbol({symbolRef}, C);
    }
  }
  // When the function ends, we report every improper value.
  ImproperMapTy improperMap = State->get<ImproperMap>();
  for (auto &pr : improperMap) {
    reportBug(C, pr.second);
  }
}

void ImproperCheckForDroppedPrivilegesChecker::reportBug(
    CheckerContext &C, ExplodedNode *N,
    std::unique_ptr<BugReporterVisitor> Visitor) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      *ImproperCheckForDroppedPrivilegesBugType,
      "Improper Check for Dropped Privileges", N);
  R->addVisitor(std::move(Visitor));
  C.emitReport(std::move(R));
}

void ento::registerImproperCheckForDroppedPrivilegesChecker(
    CheckerManager &mgr) {
  mgr.registerChecker<ImproperCheckForDroppedPrivilegesChecker>();
}

bool ento::shouldRegisterImproperCheckForDroppedPrivilegesChecker(
    const CheckerManager &mgr) {
  return true;
}
