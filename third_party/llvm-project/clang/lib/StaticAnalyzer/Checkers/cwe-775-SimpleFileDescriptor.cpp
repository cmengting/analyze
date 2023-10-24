//===-- SimpleFileDescriptorChecker.cpp ---------------------------*- C++ -*--//
//
// The checker that is responsible for CWE-775.
//
// The implementation is based on SimpleFileDescriptorChecker.
//
// Defines a checker for proper use of open/close APIs.
//   - If a file has been closed with close, it should not be accessed again.
//   Accessing a closed file results in undefined behavior.
//   - If a file was opened with open, it must be closed with close before
//   the execution ends. Failing to do so results in a resource leak.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <utility>

using namespace clang;
using namespace ento;

namespace {
typedef SmallVector<SymbolRef, 2> SymbolVector;

struct FileDescriptorState {
private:
  enum Kind { Opened, Closed } K;
  FileDescriptorState(Kind InK) : K(InK) { }

public:
  bool isOpened() const { return K == Opened; }
  bool isClosed() const { return K == Closed; }

  static FileDescriptorState getOpened() { return FileDescriptorState(Opened); }
  static FileDescriptorState getClosed() { return FileDescriptorState(Closed); }

  bool operator==(const FileDescriptorState &X) const {
    return K == X.K;
  }
  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(K);
  }
};

class SimpleFileDescriptorChecker : public Checker<check::PostCall,
                                           check::PreCall,
                                           check::DeadSymbols,
                                           check::PointerEscape> {
  CallDescription OpenFn, CloseFn;

  std::unique_ptr<BugType> DoubleCloseBugType;
  std::unique_ptr<BugType> LeakBugType;

  void reportDoubleClose(SymbolRef FileDescSym,
                         const CallEvent &Call,
                         CheckerContext &C) const;

  void reportLeaks(ArrayRef<SymbolRef> LeakedFileDescriptors, CheckerContext &C,
                   ExplodedNode *ErrNode) const;

  bool guaranteedNotToCloseFile(const CallEvent &Call) const;

public:
  SimpleFileDescriptorChecker();

  /// Process open.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  /// Process close.
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

  /// Stop tracking addresses which escape.
  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                    const InvalidatedSymbols &Escaped,
                                    const CallEvent *Call,
                                    PointerEscapeKind Kind) const;
};

} // end anonymous namespace

/// The state of the checker is a map from tracked file descriptor symbols to their
/// state. Let's store it in the ProgramState.
REGISTER_MAP_WITH_PROGRAMSTATE(FileDescriptorMap, SymbolRef, FileDescriptorState)

SimpleFileDescriptorChecker::SimpleFileDescriptorChecker()
    : OpenFn("open"), CloseFn("close", 1) {
  // Initialize the bug types.
  DoubleCloseBugType.reset(
      new BugType(this, "Double close", "Unix FileDescriptor API Error"));

  // Sinks are higher importance bugs as well as calls to assert() or exit(0).
  LeakBugType.reset(
      new BugType(this, "Resource Leak", "Unix FileDescriptor API Error",
                  /*SuppressOnSink=*/true));
}

void SimpleFileDescriptorChecker::checkPostCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (!OpenFn.matches(Call))
    return;

  // Get the symbolic value corresponding to the file handle.
  SymbolRef FileDesc = Call.getReturnValue().getAsSymbol();
  if (!FileDesc)
    return;

  // Generate the next transition (an edge in the exploded graph).
  ProgramStateRef State = C.getState();
  State = State->set<FileDescriptorMap>(FileDesc, FileDescriptorState::getOpened());
  C.addTransition(State);
}

void SimpleFileDescriptorChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (!CloseFn.matches(Call))
    return;

  // Get the symbolic value corresponding to the file handle.
  SymbolRef FileDesc = Call.getArgSVal(0).getAsSymbol();
  if (!FileDesc)
    return;

  // Check if the file descriptor has already been closed.
  ProgramStateRef State = C.getState();
  const FileDescriptorState *SS = State->get<FileDescriptorMap>(FileDesc);
  if (SS && SS->isClosed()) {
    reportDoubleClose(FileDesc, Call, C);
    return;
  }

  // Generate the next transition, in which the file descriptor is closed.
  State = State->set<FileDescriptorMap>(FileDesc, FileDescriptorState::getClosed());
  C.addTransition(State);
}

static bool isLeaked(SymbolRef Sym, const FileDescriptorState &SS,
                     bool IsSymDead, ProgramStateRef State) {
  if (IsSymDead && SS.isOpened()) {
    // If a symbol is NULL, assume that open failed on this path.
    // A symbol should only be considered leaked if it is non-null.
    ConstraintManager &CMgr = State->getConstraintManager();
    ConditionTruthVal OpenFailed = CMgr.isNull(State, Sym);
    return !OpenFailed.isConstrainedTrue();
  }
  return false;
}

void SimpleFileDescriptorChecker::checkDeadSymbols(SymbolReaper &SymReaper,
                                           CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SymbolVector LeakedFileDescriptors;
  FileDescriptorMapTy TrackedFileDescriptors = State->get<FileDescriptorMap>();
  for (FileDescriptorMapTy::iterator I = TrackedFileDescriptors.begin(),
                             E = TrackedFileDescriptors.end(); I != E; ++I) {
    SymbolRef Sym = I->first;
    bool IsSymDead = SymReaper.isDead(Sym);

    // Collect leaked symbols.
    if (isLeaked(Sym, I->second, IsSymDead, State))
      LeakedFileDescriptors.push_back(Sym);

    // Remove the dead symbol from the file descriptors map.
    if (IsSymDead)
      State = State->remove<FileDescriptorMap>(Sym);
  }

  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  if (!N)
    return;
  reportLeaks(LeakedFileDescriptors, C, N);
}

void SimpleFileDescriptorChecker::reportDoubleClose(SymbolRef FileDescSym,
                                            const CallEvent &Call,
                                            CheckerContext &C) const {
  // We reached a bug, stop exploring the path here by generating a sink.
  ExplodedNode *ErrNode = C.generateErrorNode();
  // If we've already reached this node on another path, return.
  if (!ErrNode)
    return;

  // Generate the report.
  auto R = std::make_unique<PathSensitiveBugReport>(
      *DoubleCloseBugType, "Closing a previously closed file descriptor", ErrNode);
  R->addRange(Call.getSourceRange());
  R->markInteresting(FileDescSym);
  C.emitReport(std::move(R));
}

void SimpleFileDescriptorChecker::reportLeaks(ArrayRef<SymbolRef> LeakedFileDescriptors,
                                      CheckerContext &C,
                                      ExplodedNode *ErrNode) const {
  // Attach bug reports to the leak node.
  // TODO: Identify the leaked file descriptor.
  for (SymbolRef LeakedFileDescriptor : LeakedFileDescriptors) {
    auto R = std::make_unique<PathSensitiveBugReport>(
        *LeakBugType, "Opened file is never closed; potential resource leak",
        ErrNode);
    R->markInteresting(LeakedFileDescriptor);
    C.emitReport(std::move(R));
  }
}

bool SimpleFileDescriptorChecker::guaranteedNotToCloseFile(const CallEvent &Call) const{
  // If it's not in a system header, assume it might close a file.
  if (!Call.isInSystemHeader())
    return false;

  // Handle cases where we know a buffer's /address/ can escape.
  if (Call.argumentsMayEscape())
    return false;

  // Note, even though close closes the file, we do not list it here
  // since the checker is modeling the call.

  return true;
}

// If the pointer we are tracking escaped, do not track the symbol as
// we cannot reason about it anymore.
ProgramStateRef
SimpleFileDescriptorChecker::checkPointerEscape(ProgramStateRef State,
                                        const InvalidatedSymbols &Escaped,
                                        const CallEvent *Call,
                                        PointerEscapeKind Kind) const {
  // If we know that the call cannot close a file, there is nothing to do.
  if (Kind == PSK_DirectEscapeOnCall && guaranteedNotToCloseFile(*Call)) {
    return State;
  }

  for (InvalidatedSymbols::const_iterator I = Escaped.begin(),
                                          E = Escaped.end();
                                          I != E; ++I) {
    SymbolRef Sym = *I;

    // The symbol escaped. Optimistically, assume that the corresponding file
    // handle will be closed somewhere else.
    State = State->remove<FileDescriptorMap>(Sym);
  }
  return State;
}

void ento::registerSimpleFileDescriptorChecker(CheckerManager &mgr) {
  mgr.registerChecker<SimpleFileDescriptorChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterSimpleFileDescriptorChecker(const CheckerManager &mgr) {
  return true;
}
