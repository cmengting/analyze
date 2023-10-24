/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A8_4_13/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace misra::libtooling_utils;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(const string& path, int line_number, ResultsList* results_list,
                 string str = "") {
  string error_message =
      "A std::shared_ptr shall be passed to a function as: (1) a copy to express the function shares ownership (2) an lvalue reference to express that the function replaces the managed object (3) a const lvalue reference to express that the function retains a reference count." +
      str;
  AddResultToResultsList(results_list, path, line_number, error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A8_4_13 {
namespace libtooling {

class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        parmVarDecl(
            hasAncestor(functionDecl(isDefinition()).bind("fd")),
            hasType(references(namedDecl(matchesName("::std::shared_ptr"),
                                         isExpansionInSystemHeader()))),
            unless(isExpansionInSystemHeader()))
            .bind("pvd"),
        this);
  }

  void run(const MatchFinder::MatchResult& result) {
    const ParmVarDecl* pvd = result.Nodes.getNodeAs<ParmVarDecl>("pvd");
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    if (pvd) {
      const QualType qt = pvd->getType();
      if (!qt->isReferenceType()) return;  // (1)
      ASTVisitor Visitor;
      Visitor.TraverseDecl(const_cast<FunctionDecl*>(fd));
      bool is_replaced = false, is_copied = false;
      for (const CXXMemberCallExpr* call : Visitor.getMemberCalls()) {
        const DeclRefExpr* replaced_arg = dyn_cast<DeclRefExpr>(
            call->getImplicitObjectArgument()->IgnoreImpCasts());
        if (replaced_arg && replaced_arg->getDecl() == pvd &&
            call->getMethodDecl()->getName() == "reset")
          is_replaced = true;
      }
      for (const CXXOperatorCallExpr* op : Visitor.getOperatorCalls()) {
        if (op->isAssignmentOp()) {
          const DeclRefExpr* replaced_arg =
              dyn_cast<DeclRefExpr>(op->getArg(0)->IgnoreImpCasts());
          const DeclRefExpr* copied_arg =
              dyn_cast<DeclRefExpr>(op->getArg(1)->IgnoreImpCasts());
          if (copied_arg && replaced_arg &&
              replaced_arg->getDecl() == copied_arg->getDecl())
            continue;  // self-assignment
          if (replaced_arg && replaced_arg->getDecl() == pvd)
            is_replaced = true;
          if (copied_arg && copied_arg->getDecl() == pvd) is_copied = true;
        }
      }
      for (const CXXConstructExpr* cce : Visitor.getConstructExprs()) {
        if (cce->getNumArgs() == 1) {
          const DeclRefExpr* copied_arg =
              dyn_cast<DeclRefExpr>(cce->getArg(0)->IgnoreImpCasts());
          if (copied_arg && copied_arg->getDecl() == pvd) is_copied = true;
        }
      }
      if (qt->isLValueReferenceType()) {
        if (is_replaced) return;  // (2)
        if (qt.getNonReferenceType().isConstQualified() && is_copied)
          return;  // (3)
      }
      ReportError(GetFilename(pvd, result.SourceManager),
                  GetLine(pvd, result.SourceManager), results_list_);
    }
  }

 private:
  ResultsList* results_list_;
};

void Checker::Init(ResultsList* result_list) {
  results_list_ = result_list;
  callback_ = new Callback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A8_4_13
}  // namespace autosar
