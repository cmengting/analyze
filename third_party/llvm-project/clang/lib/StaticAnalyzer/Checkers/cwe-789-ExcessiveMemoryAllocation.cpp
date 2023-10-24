//= ExcessiveMemoryAllocChecker.cpp ---------------------------*- C++ -*--//
//
// The checker that is responsible for CWE-789.
//
// Defines a checker to detect excessive memory allocation
//   - If malloc(), alloca(), realloc(), calloc() is allocating a memory space
//   greater than MaximumAllowedSize, report the problem
//   - If an array declared with size greater than MaximumAllowedSize, report
//   the problem
//
// cwe.ExcessiveMemoryAllocation:MaximumAllowedSize: clang option that specifies
// the maximum size of memory allocation allowed in MiB. Default to 4096.
//
//===----------------------------------------------------------------------===//

#include "InterCheckerAPI.h"
#include "clang/Basic/CharInfo.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "clang/include/clang/StaticAnalyzer/Core/Checker.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <functional>
#include <stdexcept>

using namespace clang;
using namespace ento;
using namespace std::placeholders;

namespace {
constexpr llvm::StringLiteral MsgUnboundedMallocArg =
    "Arg used by malloc() is not always in [0, MaximumAllowedSize] "
    "(CWE-789: Memory Allocation with Excessive Size Value)";
constexpr llvm::StringLiteral MsgUnboundedStackArray =
    "Size of stack array is not always in [0, MaximumAllowedSize] "
    "(CWE-789: Memory Allocation with Excessive Size Value)";

class ExcessiveMemoryAllocChecker
    : public Checker<check::PreCall, check::PreStmt<DeclStmt>> {

  static SVal getSizeOfArray(CheckerContext &C, ProgramStateRef state,
                             const MemRegion *R) {
    return getDynamicExtent(state, R, C.getSValBuilder());
  }

public:
  uint64_t MaximumAllowedSize; // In MiB
  ExcessiveMemoryAllocChecker();

  std::unique_ptr<BugType> ExcessiveMemoryAllocBugType;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;

  // SVal is considered safe if
  // - it's in the range [0, MaximumAllowedSize]
  bool isSValSafe(SVal arg, CheckerContext &C) const {
    ProgramStateRef state = C.getState();

    SValBuilder &svalBuilder = C.getSValBuilder();
    BasicValueFactory &BVF = svalBuilder.getBasicValueFactory();
    QualType sizeTy = svalBuilder.getContext().getSizeType();
    QualType origTy = arg.getType(C.getASTContext());
    SVal CastedSVal = svalBuilder.evalCast(arg, sizeTy, origTy);

    // Get value as byte, 1 MiB = 1 << 20 Byte
    llvm::APSInt Zero = BVF.getValue(0, sizeTy);
    llvm::APSInt UpperBound = BVF.getValue(MaximumAllowedSize << 20, sizeTy);

    return !state->assumeInclusiveRange(*CastedSVal.getAs<NonLoc>(), Zero,
                                        UpperBound, false);
  }

  void emitBug(CheckerContext &C, const Stmt *expr, ProgramStateRef state,
               const StringRef desc) const;
};

} // end anonymous namespace

void ExcessiveMemoryAllocChecker::checkPreCall(const CallEvent &Call,
                                               CheckerContext &C) const {
  // FIXME: Are there any other allocationfunction that we need to take care of?
  if (!Call.isGlobalCFunction("malloc") && !Call.isGlobalCFunction("calloc") &&
      !Call.isGlobalCFunction("realloc") && !Call.isGlobalCFunction("alloca") &&
      !Call.isGlobalCFunction("valloc") && !Call.isGlobalCFunction("pvalloc"))
    return;
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  const auto *CE = cast<CallExpr>(Call.getOriginExpr());
  if (!CE)
    return;
  auto allocSize = [&]() -> SVal {
    if (Call.isGlobalCFunction("malloc") || Call.isGlobalCFunction("alloca") ||
        Call.isGlobalCFunction("valloc") || Call.isGlobalCFunction("pvalloc"))
      return state->getSVal(CE->getArg(0), LCtx);
    if (Call.isGlobalCFunction("calloc")) {
      // calloc(size_t nmemb, size_t size) allocates nmemb * size bytes,
      // so we check the product
      auto nmemb = state->getSVal(CE->getArg(0), LCtx);
      auto size = state->getSVal(CE->getArg(1), LCtx);
      SValBuilder &svalBuilder = C.getSValBuilder();
      return svalBuilder.evalBinOpNN(
          C.getState(), BO_Mul, *nmemb.getAs<NonLoc>(), *size.getAs<NonLoc>(),
          svalBuilder.getContext().getSizeType());
    }
    if (Call.isGlobalCFunction("realloc"))
      return state->getSVal(CE->getArg(1), LCtx);

    return UnknownVal(); // It shouldn't reach here.
  }();

  if (!isSValSafe(allocSize, C))
    emitBug(C, CE, state, MsgUnboundedMallocArg);
}

void ExcessiveMemoryAllocChecker::checkPreStmt(const DeclStmt *DS,
                                               CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  for (const auto *I : DS->decls()) {
    const VarDecl *D = dyn_cast<VarDecl>(I);
    if (!D || D->getType().isNull())
      continue;

    if (!D->getType()->isArrayType())
      continue;
    const auto *DType = dyn_cast<ArrayType>(D->getType());
    if (!DType)
      continue;

    // Get buffer size for an declared array
    auto bufferSize = [&]() -> SVal {
      if (const auto *VLA = dyn_cast<VariableArrayType>(DType)) {
        // For case like char c[a-b];
        // VLA->getSizeExpr() only returns the number or elements
        // We need to multiply with the type size
        SValBuilder &svalBuilder = C.getSValBuilder();
        ASTContext &astContext = svalBuilder.getContext();
        SVal NumElement = C.getSVal(VLA->getSizeExpr());
        NonLoc TypeSize = svalBuilder.makeArrayIndex(
            astContext.getTypeSizeInChars(VLA->getElementType())
                .getQuantity()); // learned from ArrayBoundCheckerV2
        return svalBuilder.evalBinOpNN(C.getState(), BO_Mul,
                                       *NumElement.getAs<NonLoc>(), TypeSize,
                                       astContext.getSizeType());
      } else {
        // For case like char c[5];
        // getSizeOfArray() returns exactly the size of array in byte
        Loc VarLoc = state->getLValue(D, C.getLocationContext());
        const MemRegion *MR = VarLoc.getAsRegion();
        return getSizeOfArray(C, state, MR);
      }
    }();

    if (!isSValSafe(bufferSize, C))
      emitBug(C, DS, state, MsgUnboundedStackArray);
  }
}

void ExcessiveMemoryAllocChecker::emitBug(CheckerContext &C, const Stmt *expr,
                                          ProgramStateRef state,
                                          const StringRef desc) const {
  if (ExplodedNode *ErrNode = C.generateErrorNode()) {
    auto R = std::make_unique<PathSensitiveBugReport>(
        *ExcessiveMemoryAllocBugType, desc, ErrNode);
    R->addRange(expr->getSourceRange());
    C.emitReport(std::move(R));
  }
}

ExcessiveMemoryAllocChecker::ExcessiveMemoryAllocChecker() {
  ExcessiveMemoryAllocBugType.reset(
      new BugType(this, "Memory Allocation with Excessive Size Value",
                  "Unix String API Error"));
}

void ento::registerExcessiveMemoryAllocChecker(CheckerManager &Mgr) {
  ExcessiveMemoryAllocChecker *checker =
      Mgr.registerChecker<ExcessiveMemoryAllocChecker>();
  checker->MaximumAllowedSize =
      (size_t)Mgr.getAnalyzerOptions().getCheckerIntegerOption(
          checker, "MaximumAllowedSize", false);
}

bool ento::shouldRegisterExcessiveMemoryAllocChecker(
    const CheckerManager &mgr) {
  return true;
}
