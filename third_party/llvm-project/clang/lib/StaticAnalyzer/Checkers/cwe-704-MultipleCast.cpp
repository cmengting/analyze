//===-- MultipleCastChecker.cpp -----------------------------------------*- C++ -*--//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <string>
#include <utility>

using namespace clang;
using namespace ento;

namespace {
class MultipleCastChecker : public Checker<check::PostCall,
                                           check::PostStmt<CastExpr>> {
  CallDescription MallocFn;

  std::unique_ptr<BuiltinBug> MultipleCastBugType;

public:
  MultipleCastChecker();

  /// Process malloc.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostStmt(CastExpr const* CE, CheckerContext &C) const;
  void reportMultipleCast(SymbolRef Sym, CheckerContext &C, ExplodedNode *N) const;
};

} // end anonymous namespace

/// The state of the checker is a map from tracked stream symbols to their
/// state. Let's store it in the ProgramState.
REGISTER_SET_WITH_PROGRAMSTATE(AllocationSet, SymbolRef)
REGISTER_MAP_WITH_PROGRAMSTATE(CastMap, SymbolRef, QualType)

MultipleCastChecker::MultipleCastChecker()
    : MallocFn("malloc", 1) {
  // Initialize the bug types.
  MultipleCastBugType.reset(
      new BuiltinBug(this, "Incorrect Type Conversion or Cast"));
}

void MultipleCastChecker::checkPostCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (!MallocFn.matches(Call))
    return;

  // Get the symbolic value corresponding to the malloc'd variable.
  SymbolRef RegionSym = Call.getReturnValue().getAsSymbol();
  if (!RegionSym)
    return;

  // Generate the next transition (an edge in the exploded graph).
  ProgramStateRef State = C.getState();
  State = State->add<AllocationSet>(RegionSym);
  C.addTransition(State);
}

void MultipleCastChecker::checkPostStmt(CastExpr const* CE,
                                        CheckerContext &C) const {
  // skip the node that is part of explicit cast
  if (CE->getCastKind() == clang::CK_LValueToRValue) {
    return;
  }

  Expr const* CastedExpr = CE->getSubExpr();
  SymbolRef CastedSym = C.getSVal(CastedExpr).getAsSymbol();
  if (!CastedSym) {
    return;
  }
  ProgramStateRef State = C.getState();
  if (!State->contains<AllocationSet>(CastedSym)) {
    return;
  }
  QualType const* firstDestType = State->get<CastMap>(CastedSym);
  QualType destType = CE->getType();

  if (!destType->isPointerType()) {
    return;
  }

  if (!firstDestType) {
    State = State->set<CastMap>(CastedSym, destType);
    C.addTransition(State);
  } else if (*firstDestType != destType) {
    ExplodedNode *N = C.generateNonFatalErrorNode(State);
    if (!N) {
      return;
    }
    reportMultipleCast(CastedSym, C, N);
  }
}

void MultipleCastChecker::reportMultipleCast(SymbolRef Sym, CheckerContext &C, ExplodedNode *N) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
    *MultipleCastBugType, "Incorrect Type Conversion or Cast",
    N);
  R->markInteresting(Sym);
  C.emitReport(std::move(R));
}

void ento::registerMultipleCastChecker(CheckerManager &mgr) {
  mgr.registerChecker<MultipleCastChecker>();
}

bool ento::shouldRegisterMultipleCastChecker(const CheckerManager &mgr) {
  return true;
}
