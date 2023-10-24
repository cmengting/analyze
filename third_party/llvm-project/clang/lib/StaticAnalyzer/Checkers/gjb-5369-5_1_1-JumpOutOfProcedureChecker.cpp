//=== gjb-5369-5_1_1-JumpOutOfProcedure.cpp - Check whether jumping out of procedure
//-*-
// C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// The checker that is responsible for rule 5.1.1.
//
// The only way to jump out of a procedure in C program is to use longjmp function.
// Specifically, to jump out of a procedure, we need first setjmp for some jmp_buf
// variable outside the procedure, then we call longjmp and we'll jump to where we
// setjmp.
//
// In our solution, we use a JmpBuf map to keep the mapping between jmp_buf variable
// and the stack context where the setjmp is called.
//
// The general process is:
//  In Function checkPostCall():
//   1. Match setjmp() and get its first argument (jmp_buf variable)
//   2. Get the current stack context, add new records to JmpBuf map
//  In Function checkPreCall():
//   1. Match longjmp() and get its first argument (jmp_buf variable)
//   2. Check whether this jmp_buf variable is recorded in JmpBuf map previously,
//      if it is, we get the current stack context and we compare it with the context
//      when we call setjmp
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

struct StackFrame {
 private:
  StackFrameContext context;
  StackFrame(StackFrameContext InContext) : context(InContext) {}

 public:
  StackFrameContext getStackFrameContext() const { return context; }

  static StackFrame getStackFrameStruct(StackFrameContext InContext) { return StackFrame(InContext); }

  bool operator==(const StackFrame &X) const { return context.getID() == X.context.getID(); }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddPointer(&context); }
};

struct JmpBuf {
 private:
  std::string name;
  JmpBuf(std::string InName) : name(InName) {}

 public:
  std::string getJmpBufName() const { return name; }

  static JmpBuf getJmpBufStruct(std::string InName) { return JmpBuf(InName); }

  bool operator==(const JmpBuf &X) const { return name == X.name; }
  bool operator<(const JmpBuf &X) const { return name < X.name; }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddString(name); }
};

class JumpOutOfProcedureChecker : public Checker<check::PostCall, check::PreCall> {
  CallDescription SetjmpFn, LongjmpFn;

  bool isInSameContext(const StackFrameContext*, const StackFrameContext*) const;

  mutable std::unique_ptr<BugType> BT;

  void reportBug(const CallEvent &Call, CheckerContext &C) const {
    if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
      if (!BT)
        BT.reset(new BuiltinBug(this, "[gjb-5369-5_1_1]",
                                "violation of gjb5369: rule_5_1_1"));
      auto R = std::make_unique<PathSensitiveBugReport>(
          *BT, BT->getDescription(), N);
      R->addRange(Call.getSourceRange());
      C.emitReport(std::move(R));
    }
  }

 public:
  JumpOutOfProcedureChecker();

  /// Process setjmp.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  /// Process longjmp.
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};
}  // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(JmpBufMap, JmpBuf, StackFrame)

JumpOutOfProcedureChecker::JumpOutOfProcedureChecker()
    : SetjmpFn({CDF_MaybeBuiltin, "setjmp", 1}), LongjmpFn({CDF_MaybeBuiltin, "longjmp", 2}) {}

void JumpOutOfProcedureChecker::checkPostCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!Call.isGlobalCFunction()) return;

  if (!SetjmpFn.matches(Call)) return;

  // Generate the next transition (an edge in the exploded graph).
  ProgramStateRef State = C.getState();
  const SVal JmpBufArg = Call.getArgSVal(0);
  if (JmpBufArg.isUnknownOrUndef()) {
    return;
  }

  std::string JmpBufArgString = JmpBufArg.getAsRegion()->getBaseRegion()->getString();
  StackFrameContext* context = const_cast<StackFrameContext*>(Call.getLocationContext()->getStackFrame());
  State = State->set<JmpBufMap>(JmpBuf::getJmpBufStruct(JmpBufArgString),
    StackFrame::getStackFrameStruct(*context));
  C.addTransition(State);
}

void JumpOutOfProcedureChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  if (!Call.isGlobalCFunction()) return;

  if (!LongjmpFn.matches(Call)) return;

  // Get the symbolic value corresponding to the jmp buf.
  const SVal JmpBufArg = Call.getArgSVal(0);
  if (JmpBufArg.isUnknownOrUndef()) {
    return;
  }

  std::string JmpBufArgString = JmpBufArg.getAsRegion()->getBaseRegion()->getString();
  ProgramStateRef State = C.getState();
  const StackFrame* frame = State->get<JmpBufMap>(JmpBuf::getJmpBufStruct(JmpBufArgString));
  if (!frame) {
    return;
  }

  StackFrameContext context = frame->getStackFrameContext();
  if (!isInSameContext(&context, Call.getLocationContext()->getStackFrame())) {
    reportBug(Call, C);
  }

  // Generate the next transition.
  C.addTransition(State);
}

bool JumpOutOfProcedureChecker::isInSameContext(const StackFrameContext* context1, const StackFrameContext* context2) const {
  return context1->getID() == context2->getID();
}

void ento::registerJumpOutOfProcedureChecker(CheckerManager &mgr) {
  auto *Checker = mgr.registerChecker<JumpOutOfProcedureChecker>();
}

bool ento::shouldRegisterJumpOutOfProcedureChecker(const CheckerManager &mgr) {
  return true;
}
