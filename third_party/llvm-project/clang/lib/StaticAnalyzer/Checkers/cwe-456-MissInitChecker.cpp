//== MissInitChecker.cpp -------------------------------------------------*- C++
//-*--==//
//
// Checker for cwe 456
// This defines MissInitChecker, a checker that performs check
// for miss initialization by checking C standard library return value.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class MissInitChecker : public Checker<eval::Call> {
  CallDescription SetvbufFn = {"setvbuf", 4};
  // The map value (int) is the start position of pointers in the function.
  CallDescriptionMap<int> ScanfFns = {
      {{"scanf"}, 1}, {{"sscanf"}, 2}, {{"fscanf"}, 2}};
  // SprintfFns is not a map, the int value is the number of arguments.
  // No int value means the function has variable number of arguments.
  CallDescriptionSet SprintfFns = {
      {"sprintf"}, {"snprintf"}, {"vsprintf", 3}, {"vsnprintf", 4}};

  bool NeedInitExpr(const Expr *E, CheckerContext &C) const;
  std::tuple<SVal, SVal> TryInitExpr(const Expr *E, CheckerContext &C) const;
  void checkScanfAndSplitStates(const CallEvent &Call, CheckerContext &C) const;
  bool checkIntReturn(const CallEvent &Call, CheckerContext &C, int index,
                      std::function<std::pair<ProgramStateRef, ProgramStateRef>(
                          ProgramStateRef, DefinedOrUnknownSVal)>
                          stateSpiltFunc) const;

public:
  MissInitChecker();
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};
} // end anonymous namespace

MissInitChecker::MissInitChecker() {}

// Check whether this expr needs to be initialized
bool MissInitChecker::NeedInitExpr(const Expr *E, CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SValBuilder &SVB = C.getSValBuilder();
  const LocationContext *LCtx = C.getLocationContext();
  QualType PType = E->getType();
  if (!isa<PointerType>(PType))
    return false;
  QualType ValType = dyn_cast<PointerType>(PType)->getPointeeType();
  SVal LC = C.getSVal(E);
  const MemRegion *MR = LC.getAsRegion();
  if (!MR)
    return false;
  SVal val = State->getSVal(MR);
  // If the value of the pointer is already defined, just skip
  if (!val.isUndef())
    return false;
  return true;
}

// The Expr *E should be a pointer
// The return value is (Loc, SymbolConjured)
// Loc is the location of the expression
// SymbolConjured is the initialized symbol of the value type
std::tuple<SVal, SVal> MissInitChecker::TryInitExpr(const Expr *E,
                                                    CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SValBuilder &SVB = C.getSValBuilder();
  const LocationContext *LCtx = C.getLocationContext();
  // No need to check null pointer because of NeedInitExpr
  QualType PType = E->getType();
  QualType ValType = dyn_cast<PointerType>(PType)->getPointeeType();
  SVal LC = C.getSVal(E);
  const MemRegion *MR = LC.getAsRegion();
  SVal val = State->getSVal(MR);
  return {LC, SVB.conjureSymbolVal(E, LCtx, ValType, C.blockCount())};
}

// scanf returns EOF (-1) on fail
// scanf returns the number of variables that were successfully assigned
// successfully assigned variables are in order
// This function split the state into n + 1 states according to the return value
// For example, if scanf takes to variables (x, y) that need to be initialized.
// Here are the states:
// -1: x, y uninitialized
// 1: x initialized, y uninitialized
// 2: x initialized, y initialized
void MissInitChecker::checkScanfAndSplitStates(const CallEvent &Call,
                                               CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SValBuilder &SVB = C.getSValBuilder();
  const LocationContext *LCtx = C.getLocationContext();
  const Expr *CE = Call.getOriginExpr();

  int N = Call.getNumArgs();
  int otherArgs = *ScanfFns.lookup(Call);
  int storeArgs = N - otherArgs;
  // S stores n + 1 states, corresponding to the return value
  std::vector<ProgramStateRef> S;
  // On fail, scanf returns -1 but not 0
  // First add a state with return value -1
  S.push_back(State->BindExpr(CE, LCtx, SVB.makeIntVal(-1, CE->getType())));
  // Then add states with return value from 1 to n
  for (int j = 1; j <= storeArgs; j++) {
    S.push_back(State->BindExpr(CE, LCtx, SVB.makeIntVal(j, CE->getType())));
  }
  // The outer for loops among the variables to be assigned
  for (int i = otherArgs; i < N; i++) {
    const Expr *E = Call.getArgExpr(i);
    if (!NeedInitExpr(E, C))
      continue;
    SVal LC, currentArgVal;
    std::tie(LC, currentArgVal) = TryInitExpr(E, C);
    // The inner for loops among the (n + 1) states
    // Assign the value of this variable to (n + 1) states
    for (int j = 0; j <= storeArgs; j++) {
      // Bind the undefined value to corrected state
      SVal bv = (i - j >= otherArgs) ? UndefinedVal() : currentArgVal;
      S[j] = S[j]->bindLoc(LC, bv, LCtx);
    }
  }
  for (int j = 0; j <= storeArgs; j++) {
    C.addTransition(S[j]);
  }
}

// The last parameter is a callback function
// which describes how to split the state according to the function return value
bool MissInitChecker::checkIntReturn(
    const CallEvent &Call, CheckerContext &C, int index,
    std::function<std::pair<ProgramStateRef, ProgramStateRef>(
        ProgramStateRef, DefinedOrUnknownSVal)>
        stateSpiltFunc) const {
  ProgramStateRef State = C.getState();
  SValBuilder &SVB = C.getSValBuilder();
  const LocationContext *LCtx = C.getLocationContext();
  const Expr *CE = Call.getOriginExpr();

  auto returnSVal =
      SVB.conjureSymbolVal(CE, LCtx, CE->getType(), C.blockCount());
  State = State->BindExpr(CE, LCtx, returnSVal);
  SVal returnS = State->getSVal(CE, LCtx);
  ProgramStateRef stateOk, stateFail;
  // split the state according to the return value
  std::tie(stateFail, stateOk) = stateSpiltFunc(State, returnSVal);
  const Expr *E = Call.getArgExpr(index);
  if (!NeedInitExpr(E, C))
    return false;
  SVal LC, val;
  std::tie(LC, val) = TryInitExpr(E, C);
  // stateOk: bind the variable to be initialized to SymbolConjured
  // stateFail: bind the variable to UndefinedVal
  stateOk = stateOk->bindLoc(LC, val, LCtx);
  stateFail = stateFail->bindLoc(LC, UndefinedVal(), LCtx);
  // Add both stateOk and stateFail to the current node
  C.addTransition(stateOk);
  C.addTransition(stateFail);
  return true;
}

// return true when using our customized function evaluation
// return false when using clang default evalCall
bool MissInitChecker::evalCall(const CallEvent &Call, CheckerContext &C) const {
  if (ScanfFns.lookup(Call)) {
    checkScanfAndSplitStates(Call, C);
    return true;
  }
  // setvbuf returns 0 on success
  if (SetvbufFn.matches(Call)) {
    auto stateSpiltFunc = [](ProgramStateRef State, DefinedOrUnknownSVal Val) {
      return State->assume(Val);
    };
    return checkIntReturn(Call, C, 1, stateSpiltFunc);
  }
  // sprintf and so on returns negative number on fail
  if (SprintfFns.contains(Call)) {
    auto stateSpiltFunc = [](ProgramStateRef State, DefinedOrUnknownSVal Val) {
      // Int is 32-bit in C
      llvm::APSInt minInt = llvm::APSInt::getMinValue(32, false);
      llvm::APSInt negOne = llvm::APSInt({32, (uint64_t)-1}, false);
      return State->assumeInclusiveRange(Val, minInt, negOne);
    };
    return checkIntReturn(Call, C, 0, stateSpiltFunc);
  }
  return false;
}

void ento::registerMissInitChecker(CheckerManager &mgr) {
  mgr.registerChecker<MissInitChecker>();
}

bool ento::shouldRegisterMissInitChecker(const CheckerManager &mgr) {
  return true;
}
