//===-- LinkFollowingChecker.cpp ---------------------------*- C++ -*--//
// Description of CWE 363:

// The software checks the status of a file or directory before accessing it,
// which produces a race condition in which the file can be replaced with a link
// before the access is performed, causing the software to access the wrong
// file.

// While developers might expect that there is a very narrow time window between
// the time of check and time of use, there is still a race condition. An
// attacker could cause the software to slow down (e.g. with memory
// consumption), causing the time window to become larger. Alternately, in some
// situations, the attacker could win the race by performing a large number of
// attacks.

// Refer to: https://cwe.mitre.org/data/definitions/363.html.

// Analysis of description:

// Replacing the checked file with a link file before accessing it is the main
// action of attackers. From the perspective of developers, the file processed
// in the status-checking function is the same as what is accessed after since
// the variable representing the file path is unchanged in the literal aspect.
// However, the information (e.g. whether it is a link file) about the checked
// status is outdated because the checked file has been replaced.

// Therefore, if there are any types of status-checking functions called before
// accessing the file using the same file path, the information might be
// outdated due to the attack. Currently, we can only check function calls in
// the same parent function.

// Scope of status-checking functions:

// There are a series of status-checking functions that return information about
// a file, in the buffer pointed to by statbuf
// (https://www.ibm.com/docs/en/aix/7.2?topic=s-stat-fstat-lstat-statx-fstatx-statxat-fstatat-fullstat-ffullstat-stat64-fstat64-lstat64-stat64x-fstat64x-lstat64x-stat64xat-subroutine).
// Also, there are other functions like access(), only to check one aspect of
// file status (the permission). The scope of status-checking functions is wide
// and hard to cover, but it is easy to include more possible functions in the
// map of StatusCheckFnMap.
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

struct FileStatusCheckState {
private:
  enum Kind { Checked } K;
  FileStatusCheckState(Kind InK) : K(InK) {}

public:
  bool isChecked() const { return K == Checked; }

  static FileStatusCheckState getChecked() {
    return FileStatusCheckState(Checked);
  }

  bool operator==(const FileStatusCheckState &X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddInteger(K); }
};

class LinkFollowingChecker : public Checker<check::PostCall, check::PreCall> {

public:
  LinkFollowingChecker();

  /// Process file status check funcs.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  /// Process open funcs.
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

private:
  mutable std::unique_ptr<BugType> BT;

  // {{CallDescriptionFlags, func name, param count}, file path loc number}
  // The param count and path loc number are from the ibm doc mentioned above.
  CallDescriptionMap<int> StatusCheckFnMap = {
      {{CDF_MaybeBuiltin, "access", 2}, 0},
      {{CDF_MaybeBuiltin, "stat", 2}, 0},
      {{CDF_MaybeBuiltin, "stat64", 2}, 0},
      {{CDF_MaybeBuiltin, "lstat", 2}, 0},
      {{CDF_MaybeBuiltin, "lstat64", 2}, 0},
      {{CDF_MaybeBuiltin, "fstatat", 4}, 1},
      {{CDF_MaybeBuiltin, "statx", 4}, 0},
      {{CDF_MaybeBuiltin, "stat64x", 2}, 0},
      {{CDF_MaybeBuiltin, "statxat", 5}, 1},
      {{CDF_MaybeBuiltin, "stat64xat", 4}, 1},
      {{CDF_MaybeBuiltin, "fullstat", 3}, 0},
  };

  // {{CallDescriptionFlags, func name, param count}, file path loc number}
  CallDescriptionMap<int> OpenFnMap = {
      {{CDF_MaybeBuiltin, "open", 2}, 0},
      {{CDF_MaybeBuiltin, "fopen", 2}, 0},
  };

  void reportBug(CheckerContext &C) const {
    if (!BT) {
      BT = std::make_unique<BugType>(
          this, "Race Condition Enabling Link Following", categories::UnixAPI);
    }
    ExplodedNode *N = C.generateErrorNode();
    if (!N) {
      return;
    }
    auto R = std::make_unique<PathSensitiveBugReport>(
        *BT, "Race Condition Enabling Link Following", N);
    C.emitReport(std::move(R));
  }
};

} // end anonymous namespace

/// The state of the checker is a map from tracked file path symbols to
/// their state (whether the file status has been checked).
REGISTER_MAP_WITH_PROGRAMSTATE(FileStatusCheckMap, SymbolRef,
                               FileStatusCheckState)

LinkFollowingChecker::LinkFollowingChecker() {}

void LinkFollowingChecker::checkPostCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (Call.getNumArgs() == 0)
    return;

  const int *filePathLoc = StatusCheckFnMap.lookup(Call);
  if (!filePathLoc)
    return;

  SymbolRef FilePath = Call.getArgSVal(*filePathLoc).getAsSymbol();
  if (!FilePath)
    return;

  // Generate the next transition (an edge in the exploded graph).
  ProgramStateRef State = C.getState();
  State = State->set<FileStatusCheckMap>(FilePath,
                                         FileStatusCheckState::getChecked());
  C.addTransition(State);
}

void LinkFollowingChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  if (Call.getNumArgs() == 0)
    return;

  const int *filePathLoc = OpenFnMap.lookup(Call);
  if (!filePathLoc)
    return;

  // Get the symbolic value corresponding to the file path.
  SymbolRef FilePath = Call.getArgSVal(*filePathLoc).getAsSymbol();
  if (!FilePath)
    return;

  // Check if the file status has already been checked.
  ProgramStateRef State = C.getState();
  const FileStatusCheckState *SS = State->get<FileStatusCheckMap>(FilePath);
  if (SS && SS->isChecked()) {
    reportBug(C);
    return;
  }

  // Generate the next transition, in which the file descriptor is checked.
  State = State->set<FileStatusCheckMap>(FilePath,
                                         FileStatusCheckState::getChecked());
  C.addTransition(State);
}

void ento::registerLinkFollowingChecker(CheckerManager &mgr) {
  mgr.registerChecker<LinkFollowingChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterLinkFollowingChecker(const CheckerManager &mgr) {
  return true;
}
