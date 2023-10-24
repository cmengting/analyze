//== UncheckedReturnValueChecker.cpp - Unchecked Return Value --------------*-
// C++ -*--==//
//
// This defines UncheckedReturnValueChecker, a checker that performs checks for
// the value that should be checked.
//
//===----------------------------------------------------------------------===//
#include "Yaml.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Checkers/GenericTaintRuleParser.h"
#include "clang/StaticAnalyzer/Checkers/NaiveCStdLibFunctionsInfo.h"
#include "clang/StaticAnalyzer/Checkers/Taint.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <limits>
#include <memory>
#include <utility>

using namespace clang;
using namespace ento;
using namespace taint;

namespace {
class UncheckedReturnValueChecker
    : public Checker<check::PostCall, check::BranchCondition,
                     check::EndFunction> {
public:
  std::unique_ptr<BuiltinBug> UncheckedReturnValueBugType;
  std::vector<CallDescription> CustomProvidedFunctions;

private:
  static void addSymbol(SymbolRef symbolRef, CheckerContext &C);
  static ProgramStateRef removeSymbol(std::vector<SymbolRef> symbolRefVec,
                                      CheckerContext &C);

public:
  UncheckedReturnValueChecker();
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Condition, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
  void reportBug(CheckerContext &C, ExplodedNode *N,
                 std::unique_ptr<BugReporterVisitor> Visitor = nullptr) const;
};
} // end of anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(UncheckedMap, SymbolRef, ExplodedNode *)

void UncheckedReturnValueChecker::addSymbol(SymbolRef symbolRef,
                                            CheckerContext &C) {
  if (!symbolRef)
    return;

  ProgramStateRef State = C.getState();
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  // We should keep the exploded node where the value of the callee returns.
  State = State->set<UncheckedMap>(symbolRef, N);
  C.addTransition(State);
}

ProgramStateRef
UncheckedReturnValueChecker::removeSymbol(std::vector<SymbolRef> symbolRefVec,
                                          CheckerContext &C) {
  //  When we use if-stmt, while-stmt, and so on to check the value and delete
  //  it from the state.
  ProgramStateRef State = C.getState();
  for (auto it = symbolRefVec.begin(); it != symbolRefVec.end(); ++it) {
    SymbolRef symbolRef = *it;
    if (State->contains<UncheckedMap>(symbolRef)) {
      State = State->remove<UncheckedMap>(symbolRef);
    }
  }
  C.addTransition(State);
  return State;
}

UncheckedReturnValueChecker::UncheckedReturnValueChecker() {
  UncheckedReturnValueBugType.reset(
      new BuiltinBug(this, "Unchecked Return Value"));
}

void UncheckedReturnValueChecker::checkPostCall(const CallEvent &Call,
                                                CheckerContext &C) const {
  QualType resultTy = Call.getResultType();
  if (resultTy->isVoidType())
    return;

  bool isCustomProvidedFunction =
      std::find_if(CustomProvidedFunctions.begin(),
                   CustomProvidedFunctions.end(), [&Call](auto CD) -> bool {
                     return CD.matches(Call);
                   }) != CustomProvidedFunctions.end();
  if (!naive::ReturnValueNeedCheckFunctions.contains(Call) &&
      !isCustomProvidedFunction)
    return;

  SymbolRef resultVal = Call.getReturnValue().getAsSymbol();
  addSymbol(resultVal, C);
}

void UncheckedReturnValueChecker::checkBranchCondition(
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

void UncheckedReturnValueChecker::checkEndFunction(const ReturnStmt *RS,
                                                   CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  // When the function ends, we report every unchecked value.
  UncheckedMapTy uncheckedMap = State->get<UncheckedMap>();
  for (auto &pr : uncheckedMap) {
    reportBug(C, pr.second);
  }
}

void UncheckedReturnValueChecker::reportBug(
    CheckerContext &C, ExplodedNode *N,
    std::unique_ptr<BugReporterVisitor> Visitor) const {
  auto R = std::make_unique<PathSensitiveBugReport>(
      *UncheckedReturnValueBugType, "Unchecked Return Value", N);
  R->addVisitor(std::move(Visitor));
  C.emitReport(std::move(R));
}

void ento::registerUncheckedReturnValueChecker(CheckerManager &Mgr) {
  auto *Checker = Mgr.registerChecker<UncheckedReturnValueChecker>();
  using namespace yamlparser;
  GenericTaintRuleParser ConfigParser{Mgr};
  std::string Option{"Config"};
  StringRef ConfigFile =
      Mgr.getAnalyzerOptions().getCheckerStringOption(Checker, Option);
  llvm::Optional<TaintConfiguration> Config =
      getConfiguration<TaintConfiguration>(Mgr, Checker, Option, ConfigFile);
  if (!Config)
    return;

  GenericTaintRuleParser::RulesContTy Rules{
      ConfigParser.parseConfiguration<UncheckedReturnValueChecker>(
          Option, std::move(*Config))};

  for (auto it{Rules.begin()}; it != Rules.end(); ++it) {
    auto &vec = it->second.FilterArgs.DiscreteArgs;
    std::vector<const char *> names{it->first.getFunctionName().data()};
    llvm::ArrayRef<const char *> namesArrRef{names};
    auto push_back = [&](CallDescription &&CD) -> void {
      Checker->CustomProvidedFunctions.push_back(std::move(CD));
    };
    if (vec.empty()) {
      push_back({namesArrRef});
    } else {
      push_back({namesArrRef, vec[0]});
    }
  }
}

bool ento::shouldRegisterUncheckedReturnValueChecker(
    const CheckerManager &mgr) {
  return true;
}
