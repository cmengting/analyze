//== IncorretCheckedReturnValue.cpp - Incorrect Check of Function Return Value.
// C++ -*--==//
//
// This defines IncorrectCheckedReturnValueChecker, a checker that performs
// checks for the value that should be checked correctly.
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
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"

using namespace clang;
using namespace ento;
using namespace taint;

namespace {
class IncorrectCheckedReturnValueChecker
    : public Checker<check::PostCall, check::BranchCondition> {
  std::unique_ptr<BuiltinBug> IncorrectCheckedReturnValueBugType;

public:
  enum class CheckingType { Enumeral, Unsigned, Pointer };
  IncorrectCheckedReturnValueChecker();
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void reportBug(CheckerContext &C, ExplodedNode *N) const;

private:
  void addSymbol(SymbolRef symRef, const Expr *expr, CheckingType ty,
                 CheckerContext &C) const;
  void checkPointerOrEnumeralValue(const BinaryOperator *biOp,
                                   CheckerContext &C) const;
  void checkPointerValue(const BinaryOperator *biOp, CheckerContext &C) const;
  void checkEnumeralValue(const BinaryOperator *biOp, CheckerContext &C) const;
  void checkUnsignedValue(const BinaryOperator *biOp, const Expr *otherSide,
                          CheckerContext &C) const;
};
} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(ToBeCheckedMap, SymbolRef,
                               IncorrectCheckedReturnValueChecker::CheckingType)

// constructor
IncorrectCheckedReturnValueChecker::IncorrectCheckedReturnValueChecker() {
  IncorrectCheckedReturnValueBugType.reset(
      new BuiltinBug(this, "Incorrect Check of Function Return Value"));
}

// private method
void IncorrectCheckedReturnValueChecker::addSymbol(SymbolRef symRef,
                                                   const Expr *expr,
                                                   CheckingType ty,
                                                   CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  // If symRef is something like nonloc::ConcreteInt,
  // we use `SValBuilder::conjureSymbolVal` to create a new symbol
  // and use `ProgramState::BindExpr` to replace the old value
  // like `nonloc::ConcreteInt` with the conjured `SVal`.
  if (!symRef) {
    SValBuilder &SVB = C.getSValBuilder();
    auto conjuredSVal =
        SVB.conjureSymbolVal(expr, C.getLocationContext(), expr->getType(), C.blockCount());
    symRef = conjuredSVal.getAsSymbol();
    if (!symRef)
      return;

    State = State->BindExpr(expr, C.getLocationContext(), conjuredSVal);
  }

  State = State->set<ToBeCheckedMap>(symRef, ty);
  C.addTransition(State);
}

void IncorrectCheckedReturnValueChecker::checkPointerOrEnumeralValue(
    const BinaryOperator *biOp, CheckerContext &C) const {
  BinaryOperator::Opcode Op = biOp->getOpcode();
  if (!(Op == BO_LT || Op == BO_GT || Op == BO_LE || Op == BO_GE))
    return;
  ProgramStateRef State = C.getState();
  ExplodedNode *N = C.generateNonFatalErrorNode();
  reportBug(C, N);
}

void IncorrectCheckedReturnValueChecker::checkPointerValue(
    const BinaryOperator *biOp, CheckerContext &C) const {
  checkPointerOrEnumeralValue(biOp, C);
}

void IncorrectCheckedReturnValueChecker::checkEnumeralValue(
    const BinaryOperator *biOp, CheckerContext &C) const {
  checkPointerOrEnumeralValue(biOp, C);
}

void IncorrectCheckedReturnValueChecker::checkUnsignedValue(
    const BinaryOperator *biOp, const Expr *otherSide, CheckerContext &C) const {
  if (!otherSide->getType()->isIntegerType())
    return;

  BinaryOperator::Opcode Op = biOp->getOpcode();
  if (!(Op == BO_LT || Op == BO_GT || Op == BO_LE || Op == BO_GE ||
        Op == BO_EQ || Op == BO_NE))
    return;

  const auto *CE = dyn_cast<ImplicitCastExpr>(otherSide);
  if (!CE)
    return;

  const Expr *subExpr = CE->getSubExpr();
  if (!subExpr)
    return;

  if (!subExpr->getType()->isSignedIntegerType())
    return;

  ProgramStateRef State = C.getState();
  SValBuilder &SVB = C.getSValBuilder();
  auto subExprTy = subExpr->getType();
  auto castedExprTy = CE->getType();
  /*
    evalCast: Cast a given SVal to another SVal using given QualType's.
    We use evalCast to get the value before the cast.
  */
  SVal maybeNegativeSVal =
      SVB.evalCast(C.getSVal(otherSide), subExprTy, castedExprTy);

  if (auto otherSideNonLoc = maybeNegativeSVal.getAs<NonLoc>()) {
    QualType CondT = SVB.getConditionType();
    NonLoc Zero = SVB.makeIntVal(0, false);
    SVal compareWithZero =
        SVB.evalBinOpNN(State, BO_LT, *otherSideNonLoc, Zero, CondT);

    if (auto compareWithZeroVal =
            compareWithZero.getAs<DefinedOrUnknownSVal>()) {
      State = State->assume(*compareWithZeroVal, true);

      if (State) {
        ExplodedNode *N = C.generateNonFatalErrorNode();
        reportBug(C, N);
      }
    }
  }
}

// public method
void IncorrectCheckedReturnValueChecker::checkPostCall(
    const CallEvent &Call, CheckerContext &C) const {
  QualType resultTy = Call.getResultType();
  SymbolRef resultVal = Call.getReturnValue().getAsSymbol();
  const Expr *expr = Call.getOriginExpr();

  if (resultTy->isPointerType()) {
    addSymbol(resultVal, expr, CheckingType::Pointer, C);
  } else if (resultTy->isEnumeralType()) {
    addSymbol(resultVal, expr, CheckingType::Enumeral, C);
  } else if (resultTy->isUnsignedIntegerType()) {
    addSymbol(resultVal, expr, CheckingType::Unsigned, C);
  }
}

void IncorrectCheckedReturnValueChecker::checkBranchCondition(
    const Stmt *Condition, CheckerContext &C) const {
  if (const BinaryOperator *biOp = dyn_cast<BinaryOperator>(Condition)) {
    auto lhs = C.getSVal(biOp->getLHS()).getAsSymbol();
    auto rhs = C.getSVal(biOp->getRHS()).getAsSymbol();
    ProgramStateRef State = C.getState();

    SymbolRef symRef;
    const Expr *otherSide;
    std::tie(symRef, otherSide) = [&]() -> std::pair<SymbolRef, const Expr *> {
      if (lhs && State->contains<ToBeCheckedMap>(lhs)) {
        return {lhs, biOp->getRHS()};
      } else if (rhs && State->contains<ToBeCheckedMap>(rhs)) {
        return {rhs, biOp->getLHS()};
      } else {
        return {nullptr, nullptr};
      }
    }();

    if (symRef) {
      const CheckingType *ty = State->get<ToBeCheckedMap>(symRef);
      switch (*ty) {
      case CheckingType::Pointer:
        checkPointerValue(biOp, C);
        break;
      case CheckingType::Enumeral:
        checkEnumeralValue(biOp, C);
        break;
      case CheckingType::Unsigned:
        checkUnsignedValue(biOp, otherSide, C);
        break;
      }
    }
  }
}

void IncorrectCheckedReturnValueChecker::reportBug(CheckerContext &C,
                                                   ExplodedNode *N) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      *IncorrectCheckedReturnValueBugType,
      "Incorrect Check of Function Return Value", N);
  C.emitReport(std::move(R));
}

void ento::registerIncorrectCheckedReturnValueChecker(CheckerManager &mgr) {
  mgr.registerChecker<IncorrectCheckedReturnValueChecker>();
}

bool ento::shouldRegisterIncorrectCheckedReturnValueChecker(
    const CheckerManager &mgr) {
  return true;
}
