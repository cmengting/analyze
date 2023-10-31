//= NullTerminatorChecker.cpp ---------------------------*- C++ -*--//
//
// The checker that is responsible for CWE-170.
//
// Track length of each MemRegion and simulate string functions.
// A cstring is considered non-terminating if this length exceeds buffer size
//
//===----------------------------------------------------------------------===//

#include "InterCheckerAPI.h"
#include "clang/AST/FormatString.h"
#include "clang/Basic/CharInfo.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Checkers/NaiveCStdLibFunctionsInfo.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>
#include <string>

using namespace clang;
using namespace ento;
using namespace naive;
using namespace std::placeholders;

namespace {
// ssize_t read(int fd, void *buf, size_t count);
const int READ_FUNCTION_DEST_ARG_IDX = 1;
const int READ_FUNCTION_MAXLEN_ARG_IDX = 2;
// size_t strlen(const char *s);
const int STRLEN_FUNCTION_SRC_ARG_IDX = 0;
// char *strcpy(char *restrict dest, const char *src);
// char *strncpy(char *restrict dest, const char *restrict src, size_t n);
const int STRCPY_FUNCTION_DEST_ARG_IDX_COMMON = 0;
const int STRCPY_FUNCTION_SRC_ARG_IDX_COMMON = 1;
const int STRNCPY_FUNCTION_MAXLEN_ARG_IDX = 2;
// char *strcat(char *restrict dest, const char *restrict src);
// char *strncat(char *restrict dest, const char *restrict src, size_t n);
const int STRCAT_FUNCTION_DEST_ARG_IDX_COMMON = 0;
const int STRCAT_FUNCTION_SRC_ARG_IDX_COMMON = 1;
const int STRNCAT_FUNCTION_MAXLEN_ARG_IDX = 2;
// void *memmove(void *dest, const void *src, size_t n);
// void *memcpy(void *restrict dest, const void *restrict src, size_t n);
// void *memset(void *s, int c, size_t n);
const int MEM_FUNCTION_DEST_ARG_IDX_COMMON = 0;
const int MEM_FUNCTION_SRC_ARG_IDX_COMMON = 1;
const int MEM_FUNCTION_MAXLEN_ARG_IDX_COMMON = 2;
const int MEMSET_FUNCTION_CHAR_ARG_IDX = 1;

// In cwe170, we allow direct modification after terminator,
// like a[strlen(a)+1]='a'.
// In cwe464, we forbid any modification precisely after sentinel like above
constexpr llvm::StringLiteral MsgOverflow =
    "Potential Out-of-bound access caused by cstring function "
    "(CWE-170: Improper Null Termination)";
constexpr llvm::StringLiteral MsgNonTerminating =
    "Passing a potentially non-terminated cstring to function that needs "
    "proper termination "
    "(CWE-170: Improper Null Termination)";
constexpr llvm::StringLiteral MsgAccessAfterTerminator =
    "Given a pointer that may appear after the terminator "
    "(CWE-170: Improper Null Termination)";
constexpr llvm::StringLiteral MsgAccessAfterSentinel =
    "Attempt to access after the sentinel "
    "(CWE-464: Addition of Data Structure Sentinel)";

class NullTerminatorChecker
    : public Checker<check::PreCall, check::PreStmt<DeclStmt>, check::DeadSymbols,
                     check::Bind> {

public:
  struct CStringChecksFilter {
    bool CheckCStringAccessAfterSentinel = false;
    bool CheckCStringAccessAfterTerm = false;
    bool CheckCStringOutOfBound = false;
    bool CheckCStringNotNullTerm = false;
  } Filter;

  using StatePair = std::pair<ProgramStateRef, ProgramStateRef>;
  inline StatePair statePredicatePair(CheckerContext &C, ProgramStateRef state,
                                      SVal a, SVal b,
                                      BinaryOperatorKind bop) const {
    SValBuilder &SVB = C.getSValBuilder();

    return state->assume(
        SVB.evalBinOp(C.getState(), bop, a, b, SVB.getConditionType())
            .castAs<DefinedOrUnknownSVal>());
  }
  inline ProgramStateRef statePredicate(CheckerContext &C,
                                        ProgramStateRef state, SVal a, SVal b,
                                        BinaryOperatorKind bop) const {
    SValBuilder &SVB = C.getSValBuilder();

    return state->assume(
        SVB.evalBinOp(C.getState(), bop, a, b, SVB.getConditionType())
            .castAs<DefinedOrUnknownSVal>(),
        true);
  }
#define statePredicatePairEQ(C, state, a, b)                                   \
  statePredicatePair(C, state, a, b, BO_EQ)
#define statePredicatePairLE(C, state, a, b)                                   \
  statePredicatePair(C, state, a, b, BO_LE)
#define statePredicatePairLT(C, state, a, b)                                   \
  statePredicatePair(C, state, a, b, BO_LT)
#define statePredicatePairGE(C, state, a, b)                                   \
  statePredicatePair(C, state, a, b, BO_GE)
#define statePredicatePairGT(C, state, a, b)                                   \
  statePredicatePair(C, state, a, b, BO_GT)
#define statePredicateEQ(C, state, a, b) statePredicate(C, state, a, b, BO_EQ)
#define statePredicateLE(C, state, a, b) statePredicate(C, state, a, b, BO_LE)
#define statePredicateLT(C, state, a, b) statePredicate(C, state, a, b, BO_LT)
#define statePredicateGE(C, state, a, b) statePredicate(C, state, a, b, BO_GE)
#define statePredicateGT(C, state, a, b) statePredicate(C, state, a, b, BO_GT)

  // Consider a string is not terminating if strLength >= bufferSize
  ProgramStateRef stateImproperTerminate(CheckerContext &C,
                                         ProgramStateRef state, SVal strLength,
                                         SVal bufferSize) const {
    return statePredicateGE(C, state, strLength, bufferSize);
  }
  NullTerminatorChecker();

  std::unique_ptr<BugType> NotTerminatedBugType;

  static void *getTag() {
    static int tag;
    return &tag;
  }

  bool checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;
  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &) const;
  void checkDeadSymbols(SymbolReaper &SR, CheckerContext &C) const;

  using FnCheck = std::function<void(const NullTerminatorChecker *,
                                     CheckerContext &, const CallExpr *)>;
// CHECK_ARGS generates a forwarding call wrapper for evalUnsafeArgCommon
// _1, _2, _3 are placeholders, __VA_ARGS__ generates the non-variadic args
#define CHECK_ARGS(...)                                                        \
  std::bind(&NullTerminatorChecker::evalUnsafeArgCommon, _1, _2, _3,           \
            ArgSet{{__VA_ARGS__}, {}})

  CallDescriptionMap<FnCheck> Callbacks = {
      {{CDF_MaybeBuiltin, "memcpy", 3}, &NullTerminatorChecker::evalMemcpy},
      {{CDF_MaybeBuiltin, "memmove", 3}, &NullTerminatorChecker::evalMemmove},
      {{CDF_MaybeBuiltin, "memset", 3}, &NullTerminatorChecker::evalMemset},
      {{CDF_MaybeBuiltin, "strcat", 2}, &NullTerminatorChecker::evalStrcat},
      {{CDF_MaybeBuiltin, "strchr", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strcmp", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strcoll", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strcpy", 2}, &NullTerminatorChecker::evalStrcpy},
      {{CDF_MaybeBuiltin, "strcspn", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strlen", 1}, &NullTerminatorChecker::evalStrlen},
      {{CDF_MaybeBuiltin, "strncat", 3}, &NullTerminatorChecker::evalStrncat},
      {{CDF_MaybeBuiltin, "strncpy", 3}, &NullTerminatorChecker::evalStrncpy},
      {{CDF_MaybeBuiltin, "strpbrk", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strrchr", 2}, CHECK_ARGS(0)},
      {{CDF_MaybeBuiltin, "strspn", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strstr", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strtok", 2}, CHECK_ARGS(0, 1)},
      {{CDF_MaybeBuiltin, "strxfrm", 3}, CHECK_ARGS(1)},
      {{CDF_MaybeBuiltin, "read", 3}, &NullTerminatorChecker::evalRead},
      {{CDF_MaybeBuiltin, "readlink", 3}, &NullTerminatorChecker::evalReadlink},
      {{CDF_MaybeBuiltin, "printf"}, &NullTerminatorChecker::evalPrintf},
      {{CDF_MaybeBuiltin, "puts", 1}, CHECK_ARGS(0)},
      {{CDF_MaybeBuiltin, "snprintf"}, &NullTerminatorChecker::evalSnprintf},
  };

  FnCheck identifyCall(const CallEvent &Call, CheckerContext &C) const;
  void evalStrlen(CheckerContext &C, const CallExpr *CE) const;

  void evalMemcpy(CheckerContext &C, const CallExpr *CE) const;
  void evalMemset(CheckerContext &C, const CallExpr *CE) const;
  void evalMemmove(CheckerContext &C, const CallExpr *CE) const;
  void evalMemmoveCommon(CheckerContext &C, const CallExpr *CE) const;

  void evalCopyCommon(CheckerContext &C, const CallExpr *CE,
                      ProgramStateRef preState, SVal dstArg, SVal srcArg,
                      bool ReturnEnd, bool IsBounded,
                      bool returnPtr = true) const;

  void evalStrcat(CheckerContext &C, const CallExpr *CE) const;
  void evalStrncat(CheckerContext &C, const CallExpr *CE) const;
  void evalStrcatCommon(CheckerContext &C, const CallExpr *CE, bool ReturnEnd,
                        bool IsBounded, bool returnPtr = true) const;

  void evalStrcpy(CheckerContext &C, const CallExpr *CE) const;
  void evalStrncpy(CheckerContext &C, const CallExpr *CE) const;
  void evalStrcpyCommon(CheckerContext &C, const CallExpr *CE, bool ReturnEnd,
                        bool IsBounded, bool returnPtr = true) const;

  void evalReadlink(CheckerContext &C, const CallExpr *CE) const;
  void evalRead(CheckerContext &C, const CallExpr *CE) const;
  void evalReadCommon(CheckerContext &C, const CallExpr *CE) const;
  void evalPrintf(CheckerContext &C, const CallExpr *CE) const;
  void evalSnprintf(CheckerContext &C, const CallExpr *CE) const;

  // Utility methods
  void evalUnsafeArg(CheckerContext &C, const Expr *Arg) const;
  void evalUnsafeArgCommon(CheckerContext &C, const CallExpr *CE,
                           const ArgSet &ArgsNeedCheck) const;

  void emitUsingOpenStringBug(CheckerContext &C, const Stmt *expr,
                              ProgramStateRef state) const;
  void emitOverflowBug(CheckerContext &C, const Stmt *expr,
                       ProgramStateRef state) const;
  void emitAccessAfterTermBug(CheckerContext &C, const Stmt *expr,
                              ProgramStateRef state) const;
  void emitAccessAfterStnlBug(CheckerContext &C, const Stmt *expr,
                              ProgramStateRef state) const;
  void emitBug(CheckerContext &C, const Stmt *expr, ProgramStateRef state,
               const StringRef desc) const;

  inline SVal makeSValByOffset(CheckerContext &C, SVal val, SVal offset,
                               bool do_add = false) const {
    SValBuilder &SVB = C.getSValBuilder();

    return SVB.evalBinOp(C.getState(), do_add ? BO_Add : BO_Sub, val, offset,
                         C.getASTContext().getSizeType());
  }

  // Core methods that keep tracking memLoc->Length info
  // Measures the remaining buffer size start from the pointer
  // E.g. for char buffer: char str[50];
  // We get 45 if we want to measure the buffer size of str+5
  inline SVal getRemainingBufferSizeFromIndex(CheckerContext &C,
                                              ProgramStateRef &state,
                                              const Expr *Ex,
                                              const ElementRegion *MR) const {
    SVal srcBufSize = getBufferSize(C, MR->getSuperRegion());
    // We can only handle char array so far
    // thus one character always occupies one slot
    // So getIndex() will give the right offset

    // if index>=srcBufSize then emitbug
    if (Filter.CheckCStringOutOfBound &&
        statePredicateGE(C, state, MR->getIndex(), srcBufSize))
      emitOverflowBug(C, Ex, state);
    return makeSValByOffset(C, srcBufSize, MR->getIndex());
  }
  SVal getBufferSize(CheckerContext &C, const MemRegion *MR) const;

  // Measures the remaining cstring length start from the pointer
  inline SVal getRemainingCStringLengthFromIndex(CheckerContext &C,
                                                 ProgramStateRef &state,
                                                 const Expr *Ex,
                                                 const ElementRegion *MR,
                                                 bool emit = true) const {
    // Eval strlen(a+5)=strlen(a)-5
    // We require that 5<=strlen
    SVal srcLen = getCStringLength(C, state, Ex, MR->getSuperRegion());
    // if index>srcLen then emitbug
    if (emit && statePredicateGT(C, state, MR->getIndex(), srcLen)) {
      if (Filter.CheckCStringAccessAfterTerm)
        emitAccessAfterTermBug(C, Ex, state);
      if (Filter.CheckCStringAccessAfterSentinel)
        emitAccessAfterStnlBug(C, Ex, state);
    }

    // Make SVal for srcLen - MR->getIndex()
    return makeSValByOffset(C, srcLen, MR->getIndex());
  }
  SVal getCStringLength(CheckerContext &C, ProgramStateRef &state,
                        const Expr *Ex, const MemRegion *MR) const;
  SVal getCStringLengthForRegion(CheckerContext &C, ProgramStateRef &state,
                                 const Expr *Ex, const MemRegion *Buf) const;
  ProgramStateRef setCStringLength(ProgramStateRef state, const MemRegion *MR,
                                   SVal strLength) const;
  inline ProgramStateRef setCStringLengthWithIndex(CheckerContext &C,
                                                   ProgramStateRef state,
                                                   const ElementRegion *MR,
                                                   SVal strLength) const {
    // If we set 45 as cstring length for str+5
    // We set 50 for str
    return setCStringLength(
        state, MR->getSuperRegion(),
        makeSValByOffset(C, strLength, MR->getIndex(), true));
  }
};

} // end anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(CStringLength, const MemRegion *, SVal)

//===----------------------------------------------------------------------===//
// Individual checks and utility methods.
//===----------------------------------------------------------------------===//

SVal NullTerminatorChecker::getBufferSize(CheckerContext &C,
                                          const MemRegion *MR) const {
  if (!MR) {
    return UnknownVal();
  }
  return getDynamicExtent(C.getState(), MR, C.getSValBuilder());
  // TODO(xumuchen) Track bufferr size for VLA, we may need another PROGRAMSTATE
  // to do this, since we can only get the value at the declare time.
}

ProgramStateRef NullTerminatorChecker::setCStringLength(ProgramStateRef state,
                                                        const MemRegion *MR,
                                                        SVal strLength) const {
  assert(!strLength.isUndef() && "Attempt to set an undefined string length");

  MR = MR->StripCasts();

  switch (MR->getKind()) {
  case MemRegion::StringRegionKind:
    // FIXME: This can happen if we strcpy() into a string region. This is
    // undefined [C99 6.4.5p6], but we should still warn about it.
    return state;

  case MemRegion::SymbolicRegionKind:
  case MemRegion::AllocaRegionKind:
  case MemRegion::NonParamVarRegionKind:
  case MemRegion::ParamVarRegionKind:
  case MemRegion::FieldRegionKind:
  case MemRegion::ObjCIvarRegionKind:
  case MemRegion::ElementRegionKind: // See same reason in below
    // These are the types we can currently track string lengths for.
    break;

  default:
    // Other regions (mostly non-data) can't have a reliable C string length.
    // For now, just ignore the change.
    // FIXME: These are rare but not impossible. We should output some kind of
    // warning for things like strcpy((char[]){'a', 0}, "b");
    return state;
  }

  if (strLength.isUnknown()) {
    return state->remove<CStringLength>(MR);
  }
  return state->set<CStringLength>(MR, strLength);
}

SVal NullTerminatorChecker::getCStringLengthForRegion(
    CheckerContext &C, ProgramStateRef &state, const Expr *Ex,
    const MemRegion *MR) const {
  const SVal *len = state->get<CStringLength>(MR);
  if (!len) {
    SValBuilder &svalBuilder = C.getSValBuilder();
    QualType sizeTy = svalBuilder.getContext().getSizeType();
    SVal strLength = svalBuilder.getMetadataSymbolVal(
        NullTerminatorChecker::getTag(), MR, Ex, sizeTy, C.getLocationContext(),
        C.blockCount());
    state = state->set<CStringLength>(MR, strLength);
    return strLength;
  }
  return *len;
}

SVal NullTerminatorChecker::getCStringLength(CheckerContext &C,
                                             ProgramStateRef &state,
                                             const Expr *Ex,
                                             const MemRegion *MR) const {
  if (!MR) {
    return UnknownVal();
  }
  MR = MR->StripCasts();

  switch (MR->getKind()) {
  case MemRegion::StringRegionKind: {
    // Modifying the contents of string regions is undefined [C99 6.4.5p6],
    // so we can assume that the byte length is the correct C string length.
    SValBuilder &svalBuilder = C.getSValBuilder();
    QualType sizeTy = svalBuilder.getContext().getSizeType();
    const StringLiteral *strLit = cast<StringRegion>(MR)->getStringLiteral();
    return svalBuilder.makeIntVal(strLit->getLength(), sizeTy);
  }
  case MemRegion::SymbolicRegionKind:
  case MemRegion::AllocaRegionKind:
  case MemRegion::NonParamVarRegionKind:
  case MemRegion::ParamVarRegionKind:
  case MemRegion::FieldRegionKind:
  case MemRegion::ObjCIvarRegionKind:
  case MemRegion::ElementRegionKind: // For array like char a[2][20]
                                     // we can track a[0] and a[1] separately
    return getCStringLengthForRegion(C, state, Ex, MR);
  case MemRegion::CompoundLiteralRegionKind:
    // FIXME: Can we track this? Is it necessary?
    return UnknownVal();
  default:
    // Other regions (mostly non-data) can't have a reliable C string length.
    // In this case, an error is emitted and UndefinedVal is returned.
    // The caller should always be prepared to handle this case.
    return UndefinedVal();
  }
}

//===----------------------------------------------------------------------===//
// evaluation of individual function calls.
//===----------------------------------------------------------------------===//

void NullTerminatorChecker::evalMemcpy(CheckerContext &C,
                                       const CallExpr *CE) const {
  evalMemmoveCommon(C, CE);
}

void NullTerminatorChecker::evalMemset(CheckerContext &C,
                                       const CallExpr *CE) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  SVal dstArg =
      state->getSVal(CE->getArg(MEM_FUNCTION_DEST_ARG_IDX_COMMON), LCtx);
  SVal memsetArg =
      state->getSVal(CE->getArg(MEMSET_FUNCTION_CHAR_ARG_IDX), LCtx);
  SVal maxLen =
      state->getSVal(CE->getArg(MEM_FUNCTION_MAXLEN_ARG_IDX_COMMON), LCtx);

  // Functions we currently supports always return the destination
  state = state->BindExpr(CE, LCtx, dstArg, LCtx);

  const ElementRegion *dstEleMem =
      dyn_cast_or_null<ElementRegion>(dstArg.getAsRegion());
  if (!dstEleMem)
    return;

  SVal dstBufSize = getRemainingBufferSizeFromIndex(C, state, CE, dstEleMem);
  SVal dstLen = getRemainingCStringLengthFromIndex(C, state, CE, dstEleMem);

  auto memsetLambda = [&](ProgramStateRef state, bool isMemsetZero = false) {
    // If maxLen>dstBufSize then OOB
    // Example:
    //   char dest[20];
    //   memset(dest,*,22);
    if (Filter.CheckCStringOutOfBound) {
      ProgramStateRef stateOutBound, stateInBound;
      std::tie(stateOutBound, stateInBound) =
          statePredicatePairGT(C, state, maxLen, dstBufSize);
      if (stateOutBound) {
        emitOverflowBug(C, CE, state);
        return;
      }
      state = stateInBound;
    }
    if (state) {
      if (isMemsetZero) {
        // Unconditionally set dst length to 0
        // Example:
        //   char dest[100];
        //   memset(dest+100,0,2);
        SValBuilder &SVB = C.getSValBuilder();
        SVal zero = SVB.makeZeroVal(C.getASTContext().getSizeType());
        C.addTransition(setCStringLengthWithIndex(C, state, dstEleMem, zero));
      } else {
        // Now equivalent to the leaf case in strncpy
        ProgramStateRef stateBeforeTerminator, stateAfterTerminator;
        std::tie(stateBeforeTerminator, stateAfterTerminator) =
            statePredicatePairLE(C, state, maxLen, dstLen);
        // If maxLen<=dstLen then dstLen keeps
        // Example:
        //   char dest[20]="aa";
        //   memset(dest,'a',2);
        if (stateBeforeTerminator)
          C.addTransition(stateBeforeTerminator);

        // If maxLen>dstLen then reset newDstLen>=maxLen
        // Example:
        //   char dest[20]="aa";
        //   memset(dest,'a',3);
        if (stateAfterTerminator) {
          state = setCStringLength(stateAfterTerminator,
                                   dstEleMem->getSuperRegion(), UnknownVal());
          SVal newDstLen = getRemainingCStringLengthFromIndex(C, state, CE,
                                                              dstEleMem, false);
          C.addTransition(statePredicateGE(C, state, newDstLen, maxLen));
        }
      }
    }
  };

  // Zero: memset(a,0,*);
  // Non zero: memset(a,1,*);
  ProgramStateRef stateZero, stateNonZero;
  std::tie(stateNonZero, stateZero) =
      state->assume(memsetArg.castAs<DefinedOrUnknownSVal>());
  if (stateZero) {
    // Fake-zero: memset(a,0,0);
    // Real-zero: memset(a,0,1);
    ProgramStateRef stateFakeZero, stateRealZero;
    std::tie(stateRealZero, stateFakeZero) =
        state->assume(maxLen.castAs<DefinedOrUnknownSVal>());
    if (stateFakeZero)
      memsetLambda(stateFakeZero);
    if (stateRealZero)
      memsetLambda(stateRealZero, true);
  }
  if (stateNonZero)
    memsetLambda(stateNonZero);
}

void NullTerminatorChecker::evalMemmove(CheckerContext &C,
                                        const CallExpr *CE) const {
  evalMemmoveCommon(C, CE);
}

void NullTerminatorChecker::evalMemmoveCommon(CheckerContext &C,
                                              const CallExpr *CE) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  const Expr *dstArg = CE->getArg(MEM_FUNCTION_DEST_ARG_IDX_COMMON);
  const Expr *srcArg = CE->getArg(MEM_FUNCTION_SRC_ARG_IDX_COMMON);

  evalCopyCommon(C, CE, state, state->getSVal(dstArg, LCtx),
                 state->getSVal(srcArg, LCtx), /*ReturnEnd=*/false,
                 /*IsBounded=*/true, /*returnPtr=*/false);
}
void NullTerminatorChecker::evalStrcat(CheckerContext &C,
                                       const CallExpr *CE) const {
  evalStrcatCommon(C, CE, /*ReturnEnd=*/false, /*IsBounded=*/false,
                   /*returnPtr=*/false);
}

void NullTerminatorChecker::evalStrncat(CheckerContext &C,
                                        const CallExpr *CE) const {
  evalStrcatCommon(C, CE, /*ReturnEnd=*/false, /*IsBounded=*/true,
                   /*returnPtr=*/false);
}

void NullTerminatorChecker::evalStrcatCommon(CheckerContext &C,
                                             const CallExpr *CE, bool ReturnEnd,
                                             bool IsBounded,
                                             bool returnPtr) const {
  // strcat(dst,src) === strcpy(dst+strlen(dst),src)
  // strncat(dst,src) === strncpy(src+strlen(dst),src)
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  SValBuilder &SVB = C.getSValBuilder();

  const Expr *dstArg = CE->getArg(STRCAT_FUNCTION_DEST_ARG_IDX_COMMON);
  const Expr *srcArg = CE->getArg(STRCAT_FUNCTION_SRC_ARG_IDX_COMMON);

  // First check dst is terminating
  evalUnsafeArg(C, dstArg);

  // Construct dst+strlen(dst)
  SVal dstLoc = state->getSVal(dstArg, LCtx);
  const ElementRegion *dstEleMem =
      dyn_cast_or_null<ElementRegion>(dstLoc.getAsRegion());
  if (!dstEleMem)
    return;
  SVal dstLen = getCStringLength(C, state, CE, dstEleMem);
  // we need preState for evalCopyCommon
  // because we want the state change in getCStringLength here
  // to be passed to later procedure

  SVal start = SVB.evalBinOp(state, BO_Add, dstLoc, dstLen,
                             dstLoc.getType(C.getASTContext()));
  evalCopyCommon(C, CE, state, start, state->getSVal(srcArg, LCtx), ReturnEnd,
                 IsBounded, returnPtr);
}

void NullTerminatorChecker::evalStrcpy(CheckerContext &C,
                                       const CallExpr *CE) const {
  evalStrcpyCommon(C, CE, /*ReturnEnd*/ false, /*IsBounded*/ false,
                   /*returnPtr*/ false);
}

void NullTerminatorChecker::evalStrncpy(CheckerContext &C,
                                        const CallExpr *CE) const {
  evalStrcpyCommon(C, CE, /*ReturnEnd*/ false, /*IsBounded*/ true,
                   /*returnPtr*/ false);
}

void NullTerminatorChecker::evalStrcpyCommon(CheckerContext &C,
                                             const CallExpr *CE, bool ReturnEnd,
                                             bool IsBounded,
                                             bool returnPtr) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  const Expr *dstArg = CE->getArg(STRCPY_FUNCTION_DEST_ARG_IDX_COMMON);
  const Expr *srcArg = CE->getArg(STRCPY_FUNCTION_SRC_ARG_IDX_COMMON);

  evalCopyCommon(C, CE, state, state->getSVal(dstArg, LCtx),
                 state->getSVal(srcArg, LCtx), ReturnEnd, IsBounded, returnPtr);
}

void NullTerminatorChecker::evalCopyCommon(CheckerContext &C,
                                           const CallExpr *CE,
                                           ProgramStateRef preState,
                                           SVal dstArg, SVal srcArg,
                                           bool ReturnEnd, bool IsBounded,
                                           bool returnPtr) const {
  ProgramStateRef state = preState;

  const LocationContext *LCtx = C.getLocationContext();
  // Functions we currently supports always return the destination
  state = state->BindExpr(CE, LCtx, dstArg, LCtx);

  const ElementRegion *srcEleMem =
      dyn_cast_or_null<ElementRegion>(srcArg.getAsRegion());
  if (!srcEleMem)
    return;
  const ElementRegion *dstEleMem =
      dyn_cast_or_null<ElementRegion>(dstArg.getAsRegion());
  if (!dstEleMem)
    return;

  SVal srcBufSize = getRemainingBufferSizeFromIndex(C, state, CE, srcEleMem);
  SVal dstBufSize = getRemainingBufferSizeFromIndex(C, state, CE, dstEleMem);

  SVal srcLen = getRemainingCStringLengthFromIndex(C, state, CE, srcEleMem);
  SVal dstLen = getRemainingCStringLengthFromIndex(C, state, CE, dstEleMem);

  if (IsBounded) { // strncpy
    const Expr *lenArg = CE->getArg(STRNCPY_FUNCTION_MAXLEN_ARG_IDX);
    SVal maxLen = state->getSVal(lenArg, LCtx);
    auto strncpyLambda = [&](ProgramStateRef state, SVal srcLen) {
      // If srcLen<maxLen then full copy -> srcLen
      // Example:
      //   char src[10]="aaa",dest[20];
      //   strncpy(dest,src,4);
      ProgramStateRef stateFullCopy, stateTruncCopy;
      std::tie(stateFullCopy, stateTruncCopy) =
          statePredicatePairLT(C, state, srcLen, maxLen);
      if (stateFullCopy) { // Now we copy srcLen bytes
        // If srcLen>=dstBufSize then OOB
        // Example:
        //   char src[10]="aaa",dest[3];
        //   strncpy(dest,src,4);
        if (Filter.CheckCStringOutOfBound) {
          ProgramStateRef stateOutBound, stateInBound;
          std::tie(stateOutBound, stateInBound) =
              statePredicatePairGE(C, stateFullCopy, srcLen, dstBufSize);
          if (stateOutBound) {
            emitOverflowBug(C, CE, state);
            return;
          }
          state = stateInBound;
        }
        // Example:
        //   char src[10]="aaa",dest[4];
        //   strncpy(dest,src,4);
        if (state)
          C.addTransition(
              setCStringLengthWithIndex(C, state, dstEleMem, srcLen));
      }
      if (stateTruncCopy) { // Now we copy maxLen bytes

        // If maxLen>dstBufSize then OOB
        // Example:
        //   char src[100],dest[20];
        //   strncpy(dest,src,21);
        if (Filter.CheckCStringOutOfBound) {
          ProgramStateRef stateOutBound, stateInBound;
          std::tie(stateOutBound, stateInBound) =
              statePredicatePairGT(C, stateTruncCopy, maxLen, dstBufSize);
          if (stateOutBound) {
            emitOverflowBug(C, CE, state);
            return;
          }
          state = stateInBound;
        }
        if (state) {

          ProgramStateRef stateBeforeTerminator, stateAfterTerminator;
          std::tie(stateBeforeTerminator, stateAfterTerminator) =
              statePredicatePairLE(C, state, maxLen, dstLen);
          // If maxLen<=dstLen then dstLen keeps
          // Example:
          //   char src[100],dest[20]="aa";
          //   strncpy(dest,src,2);
          if (stateBeforeTerminator) {
            C.addTransition(stateBeforeTerminator);
          }
          // If maxLen>dstLen then reset newDstLen>=maxLen
          // Example:
          //   char src[100],dest[20]="aa";
          //   strncpy(dest,src,3);
          if (stateAfterTerminator) {
            state = setCStringLength(stateAfterTerminator,
                                     dstEleMem->getSuperRegion(), UnknownVal());
            SVal newDstLen = getRemainingCStringLengthFromIndex(
                C, state, CE, dstEleMem, false);
            C.addTransition(statePredicateGE(C, state, newDstLen, maxLen));
          }
        }
      }
    };

    strncpyLambda(state, srcLen);
  } else { // strcpy

    // if srcLen>=srcBufSize then improper terminate
    // Example:
    //   char src[10],dest[20];
    //   strcpy(dest,src);
    if (Filter.CheckCStringNotNullTerm &&
        stateImproperTerminate(C, state, srcLen, srcBufSize)) {
      emitUsingOpenStringBug(C, CE, state);
      return;
    }
    if (Filter.CheckCStringOutOfBound) {
      ProgramStateRef stateOutBound, stateInBound;
      std::tie(stateOutBound, stateInBound) =
          statePredicatePairGE(C, state, srcLen, dstBufSize);
      // If srcLen>=dstBufSize then OOB
      // Example:
      //   char src[10]="aaa",dest[3];
      //   strcpy(dest,src);
      if (stateOutBound) {
        emitOverflowBug(C, CE, state);
        return;
      }
      state = stateInBound;
    }
    // If srcLen<dstBufSize then full copy
    // Example:
    //   char src[10]="aaa",dest[20];
    //   strcpy(dest,src);
    if (state)
      C.addTransition(setCStringLengthWithIndex(C, state, dstEleMem, srcLen));
  }
}

void NullTerminatorChecker::evalUnsafeArg(CheckerContext &C,
                                          const Expr *Arg) const {

  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  const ElementRegion *srcMem =
      dyn_cast_or_null<ElementRegion>(state->getSVal(Arg, LCtx).getAsRegion());
  if (!srcMem)
    return;
  SVal srcBufferSize = getRemainingBufferSizeFromIndex(C, state, Arg, srcMem);

  SVal srcLength = getRemainingCStringLengthFromIndex(C, state, Arg, srcMem);
  if (Filter.CheckCStringNotNullTerm &&
      stateImproperTerminate(C, state, srcLength, srcBufferSize)) {
    emitUsingOpenStringBug(C, Arg, state);
  }
}

void NullTerminatorChecker::evalUnsafeArgCommon(
    CheckerContext &C, const CallExpr *CE, const ArgSet &ArgsNeedCheck) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  for (ArgIdxTy i = 0; i != CE->getNumArgs(); i++)
    if (ArgsNeedCheck.contains(i)) {
      evalUnsafeArg(C, CE->getArg(i));
    }
}

void NullTerminatorChecker::evalStrlen(CheckerContext &C,
                                       const CallExpr *CE) const {
  ProgramStateRef state = C.getState();
  const Expr *srcArg = CE->getArg(STRLEN_FUNCTION_SRC_ARG_IDX);
  evalUnsafeArg(C, srcArg);

  // Bind return value
  const LocationContext *LCtx = C.getLocationContext();
  const ElementRegion *srcMem = dyn_cast_or_null<ElementRegion>(
      state->getSVal(srcArg, LCtx).getAsRegion());
  if (!srcMem)
    return;
  const MemRegion *srcBufMem = srcMem->getSuperRegion();
  SVal srcLength = getRemainingCStringLengthFromIndex(C, state, CE, srcMem);
  state = state->BindExpr(CE, LCtx, srcLength);
  C.addTransition(state);
}

void NullTerminatorChecker::evalRead(CheckerContext &C,
                                     const CallExpr *CE) const {
  evalReadCommon(C, CE);
}

void NullTerminatorChecker::evalReadlink(CheckerContext &C,
                                         const CallExpr *CE) const {
  evalReadCommon(C, CE);
}

void NullTerminatorChecker::evalReadCommon(CheckerContext &C,
                                           const CallExpr *CE) const {
  ProgramStateRef state = C.getState();

  const LocationContext *LCtx = C.getLocationContext();
  const ElementRegion *bufEleMem = dyn_cast_or_null<ElementRegion>(
      state->getSVal(CE->getArg(READ_FUNCTION_DEST_ARG_IDX), LCtx)
          .getAsRegion());
  if (!bufEleMem)
    return;
  // Transit to open src
  SVal readLen = state->getSVal(CE->getArg(READ_FUNCTION_MAXLEN_ARG_IDX), LCtx);
  SVal bufSize = getRemainingBufferSizeFromIndex(C, state, CE, bufEleMem);
  SVal curLen = getRemainingCStringLengthFromIndex(C, state, CE, bufEleMem);
  // readLen > bufSize: OOB
  // Example:
  //   char a[]="aaa";
  //   read(fd,a,10);
  if (Filter.CheckCStringOutOfBound) {
    ProgramStateRef stateInBound, stateOutBound;
    std::tie(stateInBound, stateOutBound) =
        statePredicatePairLE(C, state, readLen, bufSize);
    if (stateOutBound) {
      emitOverflowBug(C, CE, state);
      return;
    }
    state = stateInBound;
  }
  if (state) {
    ProgramStateRef stateBeforeTerminator, stateAfterTerminator;
    // readLen <= curLen: keep curLen
    // Example:
    //   char a[]="aaa";
    //   read(fd,a,1);
    std::tie(stateBeforeTerminator, stateAfterTerminator) =
        statePredicatePairLE(C, state, readLen, curLen);
    if (stateBeforeTerminator) {
      C.addTransition(stateBeforeTerminator);
    }
    // readLen > curLen: reset Terminator
    // Example:
    //   char a[200]="aaa";  //curLen=3
    //   read(fd,a,100);  //readlen=100
    if (stateAfterTerminator) {
      C.addTransition(setCStringLength(
          stateAfterTerminator, bufEleMem->getSuperRegion(), UnknownVal()));
    }
  }
}

void NullTerminatorChecker::evalPrintf(CheckerContext &C,
                                       const CallExpr *CE) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  const StringLiteral *format =
      dyn_cast_or_null<StringLiteral>(CE->getArg(0)->IgnoreParenImpCasts());
  if (!format)
    return;

  class Handler : public analyze_format_string::FormatStringHandler {
    std::set<unsigned int> string_args_;
    bool HandlePrintfSpecifier(const analyze_printf::PrintfSpecifier &FS,
                               const char *startSpecifier,
                               unsigned specifierLen,
                               const TargetInfo &Target) override {
      if (FS.getConversionSpecifier().getKind() ==
          analyze_format_string::ConversionSpecifier::sArg) {
        string_args_.insert(FS.getPositionalArgIndex());
      }
      return true;
    }

  public:
    bool IsStringArg(unsigned int idx) {
      return string_args_.find(idx) != string_args_.end();
    }
    bool HasNoStringArg() { return string_args_.empty(); }
  };

  Handler H;
  analyze_format_string::ParsePrintfString(
      H, format->getString().begin(), format->getString().end(),
      C.getASTContext().getLangOpts(), C.getASTContext().getTargetInfo(),
      false);
  if (H.HasNoStringArg())
    return;

  for (int i = 0; i < CE->getNumArgs(); i++) {
    const Expr* arg = CE->getArg(i);
    if (!H.IsStringArg(i)) continue;
    if (const ElementRegion *bufVar = dyn_cast_or_null<ElementRegion>(
            state->getSVal(arg, LCtx).getAsRegion())) {

      if (const MemRegion *bufBaseMem = bufVar->getSuperRegion())
        if (state->contains<CStringLength>(bufBaseMem)) {
          evalUnsafeArg(C, arg);
        }
    }
  }
}

void NullTerminatorChecker::evalSnprintf(CheckerContext &C,
                                         const CallExpr *CE) const {
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  SVal dstArg = state->getSVal(CE->getArg(0), LCtx);
  SVal maxLenArg = state->getSVal(CE->getArg(1), LCtx);

  const ElementRegion *dstEleMem =
      dyn_cast_or_null<ElementRegion>(dstArg.getAsRegion());
  if (!dstEleMem)
    return;
  SVal dstBufSize = getRemainingBufferSizeFromIndex(C, state, CE, dstEleMem);

  ProgramStateRef maxLenIsZero, maxLenIsNonzero;
  std::tie(maxLenIsNonzero, maxLenIsZero) =
      state->assume(maxLenArg.castAs<DefinedOrUnknownSVal>());
  if (maxLenIsZero)
    return;
  if (Filter.CheckCStringOutOfBound) {
    ProgramStateRef outBound, inBound;
    std::tie(outBound, inBound) =
        statePredicatePairGT(C, state, maxLenArg, dstBufSize);
    if (outBound) {
      emitOverflowBug(C, CE, state);
      return;
    }
    state = inBound;
  }
  if (state) {
    SValBuilder &SVB = C.getSValBuilder();
    SVal one = SVB.makeIntVal(1, C.getASTContext().getSizeType());
    SVal strLen = SVB.evalBinOp(state, clang::BO_Sub, maxLenArg, one,
                                C.getASTContext().getSizeType());
    // Note: '\0' may also be written before maxlen-1, i.e.,
    // real cstringLength maybe shorter. But assuming a longer length is ok,
    // to ensure no false negative.
    state = setCStringLengthWithIndex(C, state, dstEleMem, strLen);
  }
  C.addTransition(state);
}

//===----------------------------------------------------------------------===//
// The driver method, and other Checker callbacks.
//===----------------------------------------------------------------------===//

NullTerminatorChecker::FnCheck
NullTerminatorChecker::identifyCall(const CallEvent &Call,
                                    CheckerContext &C) const {
  const auto *CE = dyn_cast_or_null<CallExpr>(Call.getOriginExpr());
  if (!CE)
    return nullptr;

  const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
  if (!FD)
    return nullptr;

  // Pro-actively check that argument types are safe to do arithmetic upon.
  // We do not want to crash if someone accidentally passes a structure
  // into, say, a C++ overload of any of these functions. We could not check
  // that for std::copy because they may have arguments of other types.
  for (const auto *I : CE->arguments()) {
    QualType T = I->getType();
    if (!T->isIntegralOrEnumerationType() && !T->isPointerType())
      return nullptr;
  }

  const FnCheck *Callback = Callbacks.lookup(Call);
  if (Callback)
    return *Callback;

  return nullptr;
}

bool NullTerminatorChecker::checkPreCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  FnCheck Callback = identifyCall(Call, C);

  // If the callee isn't a string function, let another checker handle it.
  if (!Callback)
    return false;

  // Check and evaluate the call.
  const auto *CE = cast<CallExpr>(Call.getOriginExpr());
  Callback(this, C, CE);

  // If the evaluate call resulted in no change, chain to the next eval call
  // handler.
  // Note, the custom CString evaluation calls assume that basic safety
  // properties are held. However, if the user chooses to turn off some of these
  // checks, we ignore the issues and leave the call evaluation to a generic
  // handler.
  return C.isDifferent();
}

void NullTerminatorChecker::checkPreStmt(const DeclStmt *DS,
                                         CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  // Handle initialization for char a[] = "abc";
  for (const auto *I : DS->decls()) {
    const VarDecl *D = dyn_cast<VarDecl>(I);
    if (!D)
      continue;

    // FIXME: Handle array fields of structs.
    if (!D->getType()->isArrayType())
      continue;

    Loc VarLoc = state->getLValue(D, C.getLocationContext());
    const MemRegion *MR = VarLoc.getAsRegion();
    if (!MR)
      continue;
    SVal bufferSize = getBufferSize(C, MR);

    const Expr *Init = D->getInit();
    SValBuilder &svalBuilder = C.getSValBuilder();
    QualType sizeTy = svalBuilder.getContext().getSizeType();
    if (!Init) {
      switch (D->getStorageDuration()) {
      // objects with automatic storage duration are initialized to
      // indeterminate values, and we set its cstring length to Inf
      case clang::SD_Automatic:
        state = setCStringLength(
            state, MR,
            svalBuilder.makeIntVal(
                svalBuilder.getBasicValueFactory().getMaxValue(sizeTy)));
        break;
      // objects with static and thread-local storage duration are
      // empty-initialized
      case clang::SD_Static:
      case clang::SD_Thread:
        state = setCStringLength(state, MR, svalBuilder.makeZeroVal(sizeTy));
        break;
      default:;
      }
    } else if (const StringLiteral *I = dyn_cast<StringLiteral>(Init)) {
      state = setCStringLength(state, MR,
                               svalBuilder.makeIntVal(I->getLength(), sizeTy));
    }
  }
  C.addTransition(state);
}

void NullTerminatorChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                      CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  const MemRegion *MR = Loc.getAsRegion();

  if (const ElementRegion *locEleMem = dyn_cast_or_null<ElementRegion>(MR)) {
    if (const BinaryOperator *expr = dyn_cast_or_null<BinaryOperator>(S)) {
      const MemRegion *bufMem = locEleMem->getSuperRegion();
      // Deal with bufMem[pos] = Val;

      SValBuilder &SVB = C.getSValBuilder();
      if (locEleMem->getElementType() != SVB.getContext().CharTy)
        return;
      // FIXME: is it necessary?

      SVal bufSize = getBufferSize(C, bufMem);
      SVal curLen = getCStringLength(C, state, expr->getLHS(), bufMem);
      SVal pos = locEleMem->getIndex();
      if (Filter.CheckCStringOutOfBound) {
        ProgramStateRef stateOutBound, stateInBound;
        // If pos >= bufSize then OOB
        // Example:
        //   char a[10];
        //   a[10]=0; // OOB
        std::tie(stateOutBound, stateInBound) =
            statePredicatePairGE(C, state, pos, bufSize);
        if (stateOutBound) {
          emitOverflowBug(C, S, state);
          return;
        }
        state = stateInBound;
      }
      if (state) {
        ProgramStateRef stateZero, stateNonZero;
        std::tie(stateNonZero, stateZero) =
            state->assume(Val.castAs<DefinedOrUnknownSVal>());
        if (stateZero) { // Adding terminator
          ProgramStateRef stateBeforeTerminator, stateAfterTerminator;
          std::tie(stateBeforeTerminator, stateAfterTerminator) =
              statePredicatePairLE(C, stateZero, pos, curLen);
          // pos <= curLen: update terminator as pos
          // Example:
          //   char a[10]="aaaaa";
          //   a[2]=0; // now strlen(a)=2
          if (stateBeforeTerminator) {
            C.addTransition(
                setCStringLength(stateBeforeTerminator, bufMem, pos));
          }
          // pos > curLen: do nothing
          // Example:
          //   char a[10]="aaaaa";
          //   a[7]=0; // nothing changed
          if (stateAfterTerminator) {
            C.addTransition(stateAfterTerminator);
          }
        }
        if (stateNonZero) { // May delete terminator
          ProgramStateRef stateEqTerminator, stateNeTerminator;
          std::tie(stateEqTerminator, stateNeTerminator) =
              statePredicatePairEQ(C, stateNonZero, pos, curLen);
          // pos == curLen: remove the terminator, and set newLen>pos
          // Example:
          //   char a[10]="aaaaa";
          //   a[strlen(a)]='a'; // terminator is broken
          //                     // now we only know strlen(a)>5
          if (stateEqTerminator) {
            // remove from map
            ProgramStateRef state =
                setCStringLength(stateEqTerminator, bufMem, UnknownVal());
            SVal newLen = getCStringLength(C, state, expr, bufMem);
            // set newLen > pos
            C.addTransition(statePredicateGT(C, state, newLen, pos));
          }
          // pos != curLen: nothing could be traced
          // Example:
          //   char a[10]="aaaaa";
          //   a[strlen(a)-1]='a'; // nothing changed
          //   a[strlen(a)+1]='a'; // nothing changed
          if (stateNeTerminator) {
            if (Filter.CheckCStringAccessAfterSentinel) {
              // pos > curLen: report cwe464
              if (ProgramStateRef stateAfterTerminator =
                      statePredicateGT(C, stateNeTerminator, pos, curLen))
                emitAccessAfterStnlBug(C, S, stateAfterTerminator);
              return;
            }
            C.addTransition(stateNeTerminator);
          }
        }
      }
    }
  }
}

void NullTerminatorChecker::emitUsingOpenStringBug(
    CheckerContext &C, const Stmt *expr, ProgramStateRef state) const {
  emitBug(C, expr, state, MsgNonTerminating);
}

void NullTerminatorChecker::emitOverflowBug(CheckerContext &C, const Stmt *expr,
                                            ProgramStateRef state) const {
  emitBug(C, expr, state, MsgOverflow);
}

void NullTerminatorChecker::emitAccessAfterTermBug(
    CheckerContext &C, const Stmt *expr, ProgramStateRef state) const {
  emitBug(C, expr, state, MsgAccessAfterTerminator);
}

void NullTerminatorChecker::emitAccessAfterStnlBug(
    CheckerContext &C, const Stmt *expr, ProgramStateRef state) const {
  emitBug(C, expr, state, MsgAccessAfterSentinel);
}

void NullTerminatorChecker::emitBug(CheckerContext &C, const Stmt *expr,
                                    ProgramStateRef state,
                                    const StringRef desc) const {
  if (ExplodedNode *ErrNode = C.generateErrorNode()) {
    auto R = std::make_unique<PathSensitiveBugReport>(*NotTerminatedBugType,
                                                      desc, ErrNode);
    R->addRange(expr->getSourceRange());
    C.emitReport(std::move(R));
  }
}

void NullTerminatorChecker::checkDeadSymbols(SymbolReaper &SR,
                                             CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  CStringLengthTy Entries = state->get<CStringLength>();
  if (Entries.isEmpty())
    return;

  CStringLengthTy::Factory &F = state->get_context<CStringLength>();
  for (CStringLengthTy::iterator I = Entries.begin(), E = Entries.end(); I != E;
       ++I) {
    SVal Len = I.getData();
    if (SymbolRef Sym = Len.getAsSymbol()) {
      if (SR.isDead(Sym)) {
        // CSA tends to delete my SymbolMetadata across states unexpectedly
        // So I have to mark them in use
        // FIXME: It shoule have better solution.
        if (isa<SymbolMetadata>(Sym)) {
          SR.markInUse(Sym);
        }
      }
    }
  }
}

NullTerminatorChecker::NullTerminatorChecker() {
  NotTerminatedBugType.reset(
      new BugType(this, "Not terminated cstring", "Unix String API Error"));
}

void ento::registerNullTerminatorChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<NullTerminatorChecker>();
  NullTerminatorChecker *checker = Mgr.getChecker<NullTerminatorChecker>();
  checker->Filter.CheckCStringNotNullTerm = true;
  checker->Filter.CheckCStringOutOfBound = true;
  checker->Filter.CheckCStringAccessAfterTerm = true;
}

bool ento::shouldRegisterNullTerminatorChecker(const CheckerManager &mgr) {
  return true;
}

void ento::registerAdditionOfSentinelChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<NullTerminatorChecker>();
  NullTerminatorChecker *checker = Mgr.getChecker<NullTerminatorChecker>();

  checker->Filter.CheckCStringAccessAfterSentinel = true;
}

bool ento::shouldRegisterAdditionOfSentinelChecker(const CheckerManager &mgr) {
  return true;
}
