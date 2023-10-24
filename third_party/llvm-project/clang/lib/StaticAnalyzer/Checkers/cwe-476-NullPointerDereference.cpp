//===-- NullPointerDereference.cpp ---------------------------*- C++ -*--//
//
// The checker that is responsible for CWE-476.
//
// The implementation is based on DereferenceChecker.
//
// This checker reports implicit null pointer (pointer could be null)
// while DereferenceChecker only reports explicit (pointer must be null).
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ExprObjC.h"
#include "clang/AST/ExprOpenMP.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Checkers/NaiveCStdLibFunctionsInfo.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;
using namespace naive;

namespace {
class NullDereferenceChecker
    : public Checker<check::Location, check::Bind, check::PreCall,
                     EventDispatcher<ImplicitNullDerefEvent>> {
  enum DerefKind { NullPointer, UndefinedPointerValue };
  std::set<std::string> whiteList = {"stdin", "stdout", "stderr"};

  BugType BT_Null{this, "Dereference of null pointer", categories::LogicError};
  BugType BT_Undef{this, "Dereference of undefined pointer value",
                   categories::LogicError};

  void reportBug(DerefKind K, ProgramStateRef State, const Stmt *S,
                 CheckerContext &C) const;

  bool suppressReport(CheckerContext &C, const Expr *E) const;
  void checkDereference(SVal location, const Stmt *S, CheckerContext &C) const;

public:
  void checkLocation(SVal location, bool isLoad, const Stmt* S,
                     CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  static void AddDerefSource(raw_ostream &os,
                             SmallVectorImpl<SourceRange> &Ranges,
                             const Expr *Ex, const ProgramState *state,
                             const LocationContext *LCtx,
                             bool loadedFrom = false);

  bool SuppressAddressSpaces = false;
};
} // end anonymous namespace

void
NullDereferenceChecker::AddDerefSource(raw_ostream &os,
                                   SmallVectorImpl<SourceRange> &Ranges,
                                   const Expr *Ex,
                                   const ProgramState *state,
                                   const LocationContext *LCtx,
                                   bool loadedFrom) {
  Ex = Ex->IgnoreParenLValueCasts();
  switch (Ex->getStmtClass()) {
    default:
      break;
    case Stmt::DeclRefExprClass: {
      const DeclRefExpr *DR = cast<DeclRefExpr>(Ex);
      if (const VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl())) {
        os << " (" << (loadedFrom ? "loaded from" : "from")
           << " variable '" <<  VD->getName() << "')";
        Ranges.push_back(DR->getSourceRange());
      }
      break;
    }
    case Stmt::MemberExprClass: {
      const MemberExpr *ME = cast<MemberExpr>(Ex);
      os << " (" << (loadedFrom ? "loaded from" : "via")
         << " field '" << ME->getMemberNameInfo() << "')";
      SourceLocation L = ME->getMemberLoc();
      Ranges.push_back(SourceRange(L, L));
      break;
    }
    case Stmt::ObjCIvarRefExprClass: {
      const ObjCIvarRefExpr *IV = cast<ObjCIvarRefExpr>(Ex);
      os << " (" << (loadedFrom ? "loaded from" : "via")
         << " ivar '" << IV->getDecl()->getName() << "')";
      SourceLocation L = IV->getLocation();
      Ranges.push_back(SourceRange(L, L));
      break;
    }
  }
}

static const Expr *getDereferenceExpr(const Stmt *S, bool IsBind=false){
  const Expr *E = nullptr;

  // Walk through lvalue casts to get the original expression
  // that syntactically caused the load.
  if (const Expr *expr = dyn_cast<Expr>(S))
    E = expr->IgnoreParenLValueCasts();

  if (IsBind) {
    const VarDecl *VD;
    const Expr *Init;
    std::tie(VD, Init) = parseAssignment(S);
    if (VD && Init)
      E = Init;
  }
  return E;
}

bool NullDereferenceChecker::suppressReport(CheckerContext &C,
                                        const Expr *E) const {
  // Do not report dereferences on memory that use address space #256, #257,
  // and #258. Those address spaces are used when dereferencing address spaces
  // relative to the GS, FS, and SS segments on x86/x86-64 targets.
  // Dereferencing a null pointer in these address spaces is not defined
  // as an error. All other null dereferences in other address spaces
  // are defined as an error unless explicitly defined.
  // See https://clang.llvm.org/docs/LanguageExtensions.html, the section
  // "X86/X86-64 Language Extensions"

  QualType Ty = E->getType();
  if (!Ty.hasAddressSpace())
    return false;
  if (SuppressAddressSpaces)
    return true;

  const llvm::Triple::ArchType Arch =
      C.getASTContext().getTargetInfo().getTriple().getArch();

  if ((Arch == llvm::Triple::x86) || (Arch == llvm::Triple::x86_64)) {
    switch (toTargetAddressSpace(E->getType().getAddressSpace())) {
    case 256:
    case 257:
    case 258:
      return true;
    }
  }
  return false;
}

static bool isDeclRefExprToReference(const Expr *E) {
  if (const auto *DRE = dyn_cast<DeclRefExpr>(E))
    return DRE->getDecl()->getType()->isReferenceType();
  return false;
}

void NullDereferenceChecker::reportBug(DerefKind K, ProgramStateRef State,
                                   const Stmt *S, CheckerContext &C) const {
  const BugType *BT = nullptr;
  llvm::StringRef DerefStr1;
  llvm::StringRef DerefStr2;
  switch (K) {
  case DerefKind::NullPointer:
    BT = &BT_Null;
    DerefStr1 = " results in a null pointer dereference";
    DerefStr2 = " results in a dereference of a null pointer";
    break;
  case DerefKind::UndefinedPointerValue:
    BT = &BT_Undef;
    DerefStr1 = " results in an undefined pointer dereference";
    DerefStr2 = " results in a dereference of an undefined pointer value";
    break;
  };

  // Generate an nonfatal error node
  // We care about pointers that may be null, so the error may not be fatal
  // fatal error will stop the analyzer, but we want multiple errors
  // nonfatal node will make the analyzer able to report multiple errors
  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  if (!N)
    return;

  SmallString<100> buf;
  llvm::raw_svector_ostream os(buf);

  SmallVector<SourceRange, 2> Ranges;

  switch (S->getStmtClass()) {
  case Stmt::ArraySubscriptExprClass: {
    os << "Array access";
    const ArraySubscriptExpr *AE = cast<ArraySubscriptExpr>(S);
    AddDerefSource(os, Ranges, AE->getBase()->IgnoreParenCasts(),
                   State.get(), N->getLocationContext());
    os << DerefStr1;
    break;
  }
  case Stmt::OMPArraySectionExprClass: {
    os << "Array access";
    const OMPArraySectionExpr *AE = cast<OMPArraySectionExpr>(S);
    AddDerefSource(os, Ranges, AE->getBase()->IgnoreParenCasts(),
                   State.get(), N->getLocationContext());
    os << DerefStr1;
    break;
  }
  case Stmt::UnaryOperatorClass: {
    os << BT->getDescription();
    const UnaryOperator *U = cast<UnaryOperator>(S);
    AddDerefSource(os, Ranges, U->getSubExpr()->IgnoreParens(),
                   State.get(), N->getLocationContext(), true);
    break;
  }
  case Stmt::MemberExprClass: {
    const MemberExpr *M = cast<MemberExpr>(S);
    if (M->isArrow() || isDeclRefExprToReference(M->getBase())) {
      os << "Access to field '" << M->getMemberNameInfo() << "'" << DerefStr2;
      AddDerefSource(os, Ranges, M->getBase()->IgnoreParenCasts(),
                     State.get(), N->getLocationContext(), true);
    }
    break;
  }
  case Stmt::ObjCIvarRefExprClass: {
    const ObjCIvarRefExpr *IV = cast<ObjCIvarRefExpr>(S);
    os << "Access to instance variable '" << *IV->getDecl() << "'" << DerefStr2;
    AddDerefSource(os, Ranges, IV->getBase()->IgnoreParenCasts(),
                   State.get(), N->getLocationContext(), true);
    break;
  }
  default:
    break;
  }

  auto report = std::make_unique<PathSensitiveBugReport>(
      *BT, buf.empty() ? BT->getDescription() : buf.str(), N);

  bugreporter::trackExpressionValue(N, bugreporter::getDerefExpr(S), *report);

  for (SmallVectorImpl<SourceRange>::iterator
       I = Ranges.begin(), E = Ranges.end(); I!=E; ++I)
    report->addRange(*I);

  C.emitReport(std::move(report));
}

void NullDereferenceChecker::checkDereference(SVal l, const Stmt *S,
                                              CheckerContext &C) const {
  // Check for dereference of an undefined value.
  if (l.isUndef()) {
    const Expr *DerefExpr = getDereferenceExpr(S);
    if (!suppressReport(C, DerefExpr))
      reportBug(DerefKind::UndefinedPointerValue, C.getState(), DerefExpr, C);
    return;
  }

  DefinedOrUnknownSVal location = l.castAs<DefinedOrUnknownSVal>();

  // Check for null dereferences.
  if (!isa<Loc>(location))
    return;

  ProgramStateRef state = C.getState();

  ProgramStateRef notNullState, nullState;
  std::tie(notNullState, nullState) = state->assume(location);

  if (nullState) {
    // report all implicit null pointer dereference.
    const Expr *expr = getDereferenceExpr(S);
    if (!suppressReport(C, expr)) {
      reportBug(DerefKind::NullPointer, nullState, expr, C);
      return;
    }
  }

  // From this point forward, we know that the location is not null.
  C.addTransition(notNullState);
}

void NullDereferenceChecker::checkPreCall(const CallEvent &Call,
                                          CheckerContext &C) const {
  const std::pair<ArgSet, ArgSet> *Args = FuncArgsMayReadOrWrite.lookup(Call);
  if (!Args)
    return;
  // Evaluate the args that the function may access.
  for (ArgIdxTy i = 0; i != Call.getNumArgs(); i++) {
    auto ty = Call.getArgExpr(i)->getType();
    if (!ty->isAnyPointerType()) {
      continue;
    }
    if (!Args->first.contains(i) && !Args->second.contains(i)) {
      continue;
    }
    const Expr *S = Call.getArgExpr(i);
    if (const DeclRefExpr *Decl = dyn_cast<DeclRefExpr>(S->IgnoreImpCasts())) {
      const ValueDecl *VD = Decl->getDecl();
      if (whiteList.count(VD->getNameAsString()) > 0) {
        continue;
      }
      // Don't check function arguments inside the callee function
      // We assume that arguments are checked in caller
      if (isa<ParmVarDecl>(VD)) {
        continue;
      }
    }
    SVal l = Call.getArgSVal(i);
    checkDereference(l, S, C);
  }
}

void NullDereferenceChecker::checkLocation(SVal l, bool isLoad, const Stmt *S,
                                           CheckerContext &C) const {
  checkDereference(l, S, C);
}

void NullDereferenceChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                   CheckerContext &C) const {
  // If we're binding to a reference, check if the value is known to be null.
  if (V.isUndef())
    return;

  const MemRegion *MR = L.getAsRegion();
  const TypedValueRegion *TVR = dyn_cast_or_null<TypedValueRegion>(MR);
  if (!TVR)
    return;

  if (!TVR->getValueType()->isReferenceType())
    return;

  ProgramStateRef State = C.getState();

  ProgramStateRef StNonNull, StNull;
  std::tie(StNonNull, StNull) = State->assume(V.castAs<DefinedOrUnknownSVal>());

  if (StNull) {
    if (!StNonNull) {
      const Expr *expr = getDereferenceExpr(S, /*IsBind=*/true);
      if (!suppressReport(C, expr)) {
        reportBug(DerefKind::NullPointer, StNull, expr, C);
        return;
      }
    }

    // At this point the value could be either null or non-null.
    // Record this as an "implicit" null dereference.
    if (ExplodedNode *N = C.generateSink(StNull, C.getPredecessor())) {
      ImplicitNullDerefEvent event = {V, /*isLoad=*/true, N,
                                      &C.getBugReporter(),
                                      /*IsDirectDereference=*/true};
      dispatchEvent(event);
    }
  }

  // Unlike a regular null dereference, initializing a reference with a
  // dereferenced null pointer does not actually cause a runtime exception in
  // Clang's implementation of references.
  //
  //   int &r = *p; // safe??
  //   if (p != NULL) return; // uh-oh
  //   r = 5; // trap here
  //
  // The standard says this is invalid as soon as we try to create a "null
  // reference" (there is no such thing), but turning this into an assumption
  // that 'p' is never null will not match our actual runtime behavior.
  // So we do not record this assumption, allowing us to warn on the last line
  // of this example.
  //
  // We do need to add a transition because we may have generated a sink for
  // the "implicit" null dereference.
  C.addTransition(State, this);
}

void ento::registerNullDereferenceChecker(CheckerManager &mgr) {
  auto *Chk = mgr.registerChecker<NullDereferenceChecker>();
  Chk->SuppressAddressSpaces = mgr.getAnalyzerOptions().getCheckerBooleanOption(
      mgr.getCurrentCheckerName(), "SuppressAddressSpaces");
}

bool ento::shouldRegisterNullDereferenceChecker(const CheckerManager &mgr) {
  return true;
}
