/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A8_4_10/libtooling/checker.h"

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

void ReportError(const string& path, int line_number,
                 ResultsList* results_list) {
  string error_message =
      "A parameter shall be passed by reference if it can't be NULL.";
  AddResultToResultsList(results_list, path, line_number, error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A8_4_10 {
namespace libtooling {
// This rule is talking about a pointer parameter need to be written as
// a reference when its function is not designed to handle the situation of
// a NULL/nullptr passed in (that parameter is not optional since "no argument"
// is not a valid option).
class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        parmVarDecl(hasAncestor(functionDecl(isDefinition()).bind("fd")),
                    hasType(pointerType()), unless(isExpansionInSystemHeader()))
            .bind("pvd"),
        this);
  }

  void run(const MatchFinder::MatchResult& result) {
    const ParmVarDecl* pvd = result.Nodes.getNodeAs<ParmVarDecl>("pvd");
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    if (pvd) {
      ASTVisitor Visitor;
      Visitor.TraverseDecl(const_cast<FunctionDecl*>(fd));
      for (const IfStmt* if_stmt : Visitor.getIfStmts()) {
        const Expr* condition = if_stmt->getCond()->IgnoreImpCasts();
        if (const BinaryOperator* bo = dyn_cast<BinaryOperator>(condition)) {
          if (bo->getOpcode() == BO_EQ || bo->getOpcode() == BO_NE) {
            const Expr* lhs = bo->getLHS()->IgnoreImpCasts();
            const Expr* rhs = bo->getRHS()->IgnoreImpCasts();
            if (isCheckNULLCondition(lhs, rhs, pvd) ||
                isCheckNULLCondition(rhs, lhs, pvd))
              return;
          }
        } else if (const UnaryOperator* uo =
                       dyn_cast<UnaryOperator>(condition)) {
          if (uo->getOpcode() == UO_LNot) {
            const DeclRefExpr* operand =
                dyn_cast<DeclRefExpr>(uo->getSubExpr()->IgnoreImpCasts());
            if (operand && operand->getDecl() == pvd) return;
          }
        } else if (const DeclRefExpr* dre = dyn_cast<DeclRefExpr>(condition)) {
          if (dre->getDecl() == pvd) return;
        }
      }
      ReportError(GetFilename(pvd, result.SourceManager),
                  GetLine(pvd, result.SourceManager), results_list_);
    }
  }

 private:
  ResultsList* results_list_;
  bool isCheckNULLCondition(const Expr* operand1, const Expr* operand2,
                            const ParmVarDecl* pvd) {
    const DeclRefExpr* op1 = dyn_cast<DeclRefExpr>(operand1);
    const CXXNullPtrLiteralExpr* op2_nullptr =
        dyn_cast<CXXNullPtrLiteralExpr>(operand2);
    const GNUNullExpr* op2_NULL = dyn_cast<GNUNullExpr>(operand2);
    if (op1 && (op2_nullptr || op2_NULL) && op1->getDecl() == pvd) return true;
    return false;
  }
};

void Checker::Init(ResultsList* result_list) {
  results_list_ = result_list;
  callback_ = new Callback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A8_4_10
}  // namespace autosar
