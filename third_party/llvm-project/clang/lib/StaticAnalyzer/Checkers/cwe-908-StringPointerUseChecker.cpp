//== StringPointerUseChecker.cpp --------------------------------*- C++ -*--==//
//
// This defines StringPointerUseChecker, a checker that performs check
// for string pointer use in C standard library functions.
//
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

using namespace clang;
using namespace ento;
using namespace taint;
using namespace naive;

namespace {
class StringPointerUseChecker : public Checker< check::PreCall > {
  mutable std::unique_ptr<BuiltinBug> BT;
  void reportBug(CheckerContext &C, int pos) const;

public:
  StringPointerUseChecker();
  void checkPreCall(const CallEvent &Call,CheckerContext &C) const;
};
} // end anonymous namespace

StringPointerUseChecker::StringPointerUseChecker() {
  BT.reset(
      new BuiltinBug(this, "Uninitialized String Pointer"));
}

void StringPointerUseChecker::reportBug(CheckerContext &C, int pos) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      *BT,
      "Uninitialized String Pointer Use in Stardard Library Function on "
      "Argument " +
          std::to_string(pos),
      C.generateNonFatalErrorNode());
  C.emitReport(std::move(R));
}

void StringPointerUseChecker::checkPreCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (!Call.isGlobalCFunction()) {
    return;
  }
  // Get matched function argument list
  const ArgSet *Args = FuncCharArgsMayRead.lookup(Call);
  if (!Args) {
    return;
  }
  for (ArgIdxTy i = 0; i < Call.getNumArgs(); i++) {
    if (!Args->contains(i)) {
      continue;
    }
    // val is like str, dval is like str[0]
    // check whether str[0] is undifined
    SVal val = Call.getArgSVal(i);
    const MemRegion *MR = val.getAsRegion();
    if (!MR) {
      continue;
    }
    SVal dval = State->getSVal(MR);
    if (dval.isUnknownOrUndef()) {
      reportBug(C, i);
      break;
    }
  }
}

void ento::registerStringPointerUseChecker(CheckerManager &mgr) {
  mgr.registerChecker<StringPointerUseChecker>();
}

bool ento::shouldRegisterStringPointerUseChecker(const CheckerManager &mgr) {
  return true;
}
