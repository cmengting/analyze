//=== misrac-2012-21_19.cpp - Check returned pointer usage -*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This checker implements rule 21.19.
// The main task of this checker:
// 1. Identify calls to localeconv, getenv, strerror, and setlocale.
// 2. Check if their returned values are assigned to a const pointer.
// 3. Check if their returned values are modified.
//
// Procedure:
// 1. For task 1, this checker utilizes CallDescriptionMap in CallEvent.h. Refer
//    to CStringChecker for its usage in call matching.
// 2. After matching the function call, the checker will save the SymbolicRegion
//    of the returned value into ProgramState, which will help in detecting
//    modifications on the returned value later.
// 3. On each bind to MemRegion (checkBind), the checker will first check if the
//    value part is the return of interested functions. If yes, it will then
//    check the type of the assignment location and report an error if it is not
//    const-qualified.
//    If the location of the binding is (or from) one of the previous return of
//    interested functions (checked by checkRegionChain), it will signal an
//    error since it should not be modified.
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

using namespace clang;
using namespace clang::ento;

namespace {
class ConstPointerReturnChecker : public Checker<check::Bind, check::PostCall> {
public:
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;

private:
  mutable std::unique_ptr<BugType> BT;

  CallDescriptionMap<bool> FuncRegList = {
      {{CDF_MaybeBuiltin, "localeconv", 0}, true},
      {{CDF_MaybeBuiltin, "setlocale", 2}, true},
      {{CDF_MaybeBuiltin, "getenv", 1}, true},
      {{CDF_MaybeBuiltin, "strerror", 1}, true},
  };

  bool checkSingleBase(const SymbolicRegion *SR, ProgramStateRef state) const;
  bool checkRegionChain(const SymbolicRegion *SR, ProgramStateRef state) const;

  void reportBug(CheckerContext &C) const {
    if (!BT) {
      BT = std::make_unique<BugType>(this, "returned pointer ",
                                     categories::LogicError);
    }
    ExplodedNode *N = C.generateErrorNode();
    auto R = std::make_unique<PathSensitiveBugReport>(
        *BT,
        "[misrac-2012-21.19]: The pointers returned by the Standard Library "
        "functions localeconv, getenv, setlocale or strerror shall be const "
        "qualified and not editable.",
        N);
    C.emitReport(std::move(R));
  }
};

} // namespace

REGISTER_SET_WITH_PROGRAMSTATE(LconvObj, SymbolRef)

void ConstPointerReturnChecker::checkPostCall(const CallEvent &Call,
                                              CheckerContext &C) const {
  if (!FuncRegList.lookup(Call)) {
    return;
  }
  ProgramStateRef state = C.getState();
  SVal R = Call.getReturnValue();
  // in case self-defined functions with the same name return int value
  if (!R.getAsRegion()) {
    return;
  }
  const SymbolicRegion *SR =
      dyn_cast<SymbolicRegion>(R.getAsRegion()->getBaseRegion());
  state = state->add<LconvObj>(SR->getSymbol());

  C.addTransition(state);
}

void ConstPointerReturnChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                          CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  ASTContext &ACtx = C.getASTContext();

  const MemRegion *VMR = nullptr;
  const SymbolicRegion *VSR = nullptr;

  if ((VMR = V.getAsRegion()) &&
      (VSR = dyn_cast<SymbolicRegion>(VMR->getBaseRegion()))) {
    if (checkSingleBase(VSR, state)) {
      QualType LocTy = L.getType(ACtx)->getPointeeType();
      if (LocTy->isPointerType()) {
        if (!LocTy->getPointeeType().isConstQualified()) {
          reportBug(C);
        }
        return;
      }
      if (!LocTy.isConstQualified()) {
        reportBug(C);
        return;
      }
    }
  }

  if (!L.getAsRegion()) {
    return;
  }
  const SymbolicRegion *LSR =
      dyn_cast_or_null<SymbolicRegion>(L.getAsRegion()->getBaseRegion());

  if (checkRegionChain(LSR, state)) {
    reportBug(C);
    return;
  }
}

bool ConstPointerReturnChecker::checkSingleBase(const SymbolicRegion *SR,
                                                ProgramStateRef state) const {
  SymbolRef SRef = SR->getSymbol();
  if (state->contains<LconvObj>(SRef)) {
    return true;
  }
  return false;
}

bool ConstPointerReturnChecker::checkRegionChain(const SymbolicRegion *SR,
                                                 ProgramStateRef state) const {
  while (SR) {
    if (checkSingleBase(SR, state)) {
      return true;
    }

    SymbolRef SRef = SR->getSymbol();
    const MemRegion *OMR = SRef->getOriginRegion();
    if (OMR) {
      SR = dyn_cast_or_null<SymbolicRegion>(OMR->getBaseRegion());
    } else {
      SR = nullptr;
    }
  }
  return false;
}

void ento::registerConstPointerReturnChecker(CheckerManager &mgr) {
  auto *Checker = mgr.registerChecker<ConstPointerReturnChecker>();
}

bool ento::shouldRegisterConstPointerReturnChecker(const CheckerManager &mgr) {
  return true;
}
