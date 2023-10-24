#include "InterCheckerAPI.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "clang/include/clang/StaticAnalyzer/Core/Checker.h"
#include "llvm/ADT/SmallString.h"
#include <cstdint>
#include <functional>
#include <stdexcept>

using namespace clang;
using namespace ento;
using namespace std::placeholders;
using namespace ast_matchers;

/*
CWE-131: Incorrect Calculation of Buffer Size

This rule is only concerned with malloc(), since other types of allocation
functions like calloc() will calculate the size by the element size
automatically.

When we allocate the memory size for a buffer, we should calculate the size
carefully according to the context and the coding pattern, and may check the
value with bounds or put some restrictions on the format of the argument of
malloc(), otherwise lead to a buffer overflow.

For coding context-sensitive cases (e.g., Example 3), we can not handle it in
any static analysis way.

For specific coding patterns (e.g., sentinel value pattern in Example 1), we
have to guess whether the developer uses such a pattern by searching for a code
combination in the context. If it has such a combination, we may check this rule
by the related coding pattern with a greater chance of confirmation. Of
cause, maybe we should leave an option for users to choose whether to check.

For value checking (e.g., Example 5), we trace the argument size value and check
it with the target pointer type it allocated. If the value is not an integer
multiple of the target type size, it is non-compliant.

Restricting the format of the argument is the easiest way to check for some
simple cases (e.g., Example 4). It can be solved by simply checking the AST body
without any reasoning by a CSA or libtooling checker.

Currently, we only finished the later two simple cases:

1. checkASTCodeBody() for Example 4: Matcher for malloc() without a feasible
allocation value and emit bug reports by emitDiagnostics(), as a libtooling
checker. For the simplification of checker calling, we use checkASTCodeBody()
instead of adding another libtooling checker.

2. checkPreCall() for Example 5: Get the SVal of the argument and the target
pointer type of malloc() and check whether the allocation value is an integer
multiple of the target type size by isIntegerMultipleOfTypeSizeOrSkip().

*/

namespace {
static constexpr const char *const MallocCall = "MallocCall";

constexpr llvm::StringLiteral MsgMallocCalculationError =
    "Arg used by malloc() is not always an integer multiple of the target type "
    "element size "
    "(CWE-131: Incorrect Calculation of Buffer Size)";

class BufferSizeCalculation
    : public Checker<check::ASTCodeBody, check::PreCall> {

public:
  BufferSizeCalculation();

  std::unique_ptr<BugType> MallocCalculationErrorBugType;

  // Check for the AST struct of malloc(), which usually with a decl in its
  // argument, e.g., malloc(i * 3), malloc(size * sizeof(int)).
  // It almost the same as a Libtooling implementation.
  void checkASTCodeBody(const Decl *D, AnalysisManager &AM,
                        BugReporter &BR) const;

  // Check for malloc() with only a linear combination of int literal in its
  // argument, e.g., malloc(3), malloc(3 * 4), malloc(3 * sizeof(int)).
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  // For checkPreCall: SVal is considered safe if it's an integer multiple of
  // the target type size
  bool isIntegerMultipleOfTypeSizeOrSkip(SVal arg, QualType targetType,
                                         CheckerContext &C) const {
    SValBuilder &svalBuilder = C.getSValBuilder();
    BasicValueFactory &BVF = svalBuilder.getBasicValueFactory();
    QualType sizeTy = svalBuilder.getContext().getSizeType();

    // Get the element size of the target type.
    // divide by 8: the minimal element size in malloc is 8 bits (1 byte)
    llvm::APSInt ElementSize = BVF.getValue(
        svalBuilder.getContext().getTypeSize(targetType) / 8, sizeTy);
    if (arg.isConstant() && arg.getSubKind() == nonloc::ConcreteIntKind) {
      const llvm::APSInt &IntVal = arg.castAs<nonloc::ConcreteInt>().getValue();
      // check whether the arg is an integer multiple
      return (0 == IntVal % ElementSize);
    }
    // Skip if the arg analyzed here is not a constant and not an int value.
    return true;
  }

  void emitBug(CheckerContext &C, const Stmt *expr, const StringRef desc) const;
};

} // end anonymous namespace

// emit basic report
static void emitDiagnostics(const BoundNodes &Nodes, BugReporter &BR,
                            AnalysisDeclContext *ADC,
                            const BufferSizeCalculation *Checker) {
  const CallExpr *CE = Nodes.getNodeAs<CallExpr>(MallocCall);
  assert(CE);

  BR.EmitBasicReport(
      ADC->getDecl(), Checker,
      /*Name=*/"CWE-131", categories::SecurityError, MsgMallocCalculationError,
      PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(), ADC),
      CE->getSourceRange());
}

void BufferSizeCalculation::checkASTCodeBody(const Decl *D, AnalysisManager &AM,
                                             BugReporter &BR) const {

  AnalysisDeclContext *ADC = AM.getAnalysisDeclContext(D);

  clang::ast_matchers::internal::BindableMatcher<clang::Decl> malloc =
      functionDecl(hasName("malloc"), isExpansionInSystemHeader());

  // In malloc(a * sizeof(b)), the type of a should be unsigned int.
  clang::ast_matchers::internal::BindableMatcher<clang::Stmt>
      unsigned_int_plus_sizeof = binaryOperator(
          hasOperatorName("*"),
          hasEitherOperand(sizeOfExpr(unaryExprOrTypeTraitExpr())),
          hasEitherOperand(hasDescendant(
              declRefExpr(to(varDecl(hasType(isUnsignedInteger())))))));

  clang::ast_matchers::internal::BindableMatcher<clang::Stmt> int_plus_sizeof =
      binaryOperator(hasOperatorName("*"),
                     hasEitherOperand(sizeOfExpr(unaryExprOrTypeTraitExpr())),
                     hasEitherOperand(hasDescendant(
                         declRefExpr(to(varDecl(equalsBoundNode("intVar")))))));

  // Feasible allocation value has the following types.
  // 1) integerLiteral, e.g., malloc(3)
  // 2) with int varDecl, e.g., malloc(a), malloc(a * b)
  // 3) unsigned int * sizeof(type), e.g., malloc(a * sizeof(b))
  // Matcher for malloc() without a feasible allocation value.
  SmallVector<clang::ast_matchers::BoundNodes, 1> Matches = match(
      findAll(callExpr(callee(malloc),
                       unless(anyOf(
                           hasDescendant(integerLiteral()),
                           hasDescendant(declRefExpr(
                               to(varDecl(hasType(isInteger())).bind("intVar")),
                               unless(hasAncestor(int_plus_sizeof)))),
                           hasDescendant(unsigned_int_plus_sizeof))))
                  .bind(MallocCall)),
      *D->getBody(), AM.getASTContext());
  for (BoundNodes Match : Matches)
    emitDiagnostics(Match, BR, ADC, this);
}

// emit a path sensitive bug report
void BufferSizeCalculation::emitBug(CheckerContext &C, const Stmt *expr,
                                    const StringRef desc) const {
  if (ExplodedNode *ErrNode = C.generateErrorNode()) {
    auto R = std::make_unique<PathSensitiveBugReport>(
        *MallocCalculationErrorBugType, desc, ErrNode);
    R->addRange(expr->getSourceRange());
    C.emitReport(std::move(R));
  }
}

void BufferSizeCalculation::checkPreCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  if (!Call.isGlobalCFunction("malloc"))
    return;
  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();

  // 1) get the SVal of the arg of malloc()
  const CallExpr *CE = cast<CallExpr>(Call.getOriginExpr());
  if (!CE)
    return;
  SVal allocVal = [&]() -> SVal {
    if (Call.isGlobalCFunction("malloc"))
      return state->getSVal(CE->getArg(0), LCtx);
    return UnknownVal();
  }();
  if (isa<UnknownVal>(allocVal))
    return;

  // 2) get the target type of malloc()
  clang::DynTypedNodeList parents = C.getASTContext().getParents(*CE);
  // If its parent is not a CSyleCastExpr, the target type is not specified.
  const CStyleCastExpr *castExpr = parents[0].get<CStyleCastExpr>();
  if (castExpr == nullptr)
    return;
  QualType CastedType = castExpr->getType();
  if (!CastedType->isPointerType())
    return;
  // The pointee type should be a canonical type, otherwise it is hard to
  // calculate the target type size.
  QualType PointeeType = CastedType->getPointeeType();
  if (PointeeType->isVoidType() || !PointeeType.isCanonical())
    return;

  // 3) check whether the allocVal is an integer multiple of the target type
  // size
  if (!isIntegerMultipleOfTypeSizeOrSkip(allocVal, PointeeType, C))
    emitBug(C, CE, MsgMallocCalculationError);
}

BufferSizeCalculation::BufferSizeCalculation() {
  MallocCalculationErrorBugType.reset(new BugType(
      this, "Incorrect Calculation of Buffer Size", categories::SecurityError));
}

void ento::registerBufferSizeCalculation(CheckerManager &Mgr) {
  BufferSizeCalculation *checker = Mgr.registerChecker<BufferSizeCalculation>();
}

bool ento::shouldRegisterBufferSizeCalculation(const CheckerManager &mgr) {
  return true;
}
