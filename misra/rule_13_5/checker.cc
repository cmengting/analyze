/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_13_5/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "clang/AST/Expr.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace misra {
namespace rule_13_5 {

class CastCallback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        binaryOperator(anyOf(hasOperatorName("&&"), hasOperatorName("||")),
                       hasRHS(expr().bind("rhs"))),
        this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const Expr* rhs = result.Nodes.getNodeAs<Expr>("rhs");
    ASTContext* context = result.Context;
    clang::FullSourceLoc loc = context->getFullLoc(rhs->getBeginLoc());
    if (loc.isInvalid() || loc.isInSystemHeader()) {
      return;
    }
    string path = libtooling_utils::GetFilename(rhs, result.SourceManager);
    int line_number = libtooling_utils::GetLine(rhs, result.SourceManager);
    string location = libtooling_utils::GetLocation(rhs, result.SourceManager);
    if (rhs->HasSideEffects(*context)) {
      libtooling_utils::ConstCallExprVisitor Visitor;
      Visitor.Visit(rhs);
      if (Visitor.hasCallExpr && Visitor.hasDirectCall &&
          !Visitor.hasPersistentSideEffects)
        return;

      std::string error_message = absl::StrFormat(
          "[C1602][misra-c2012-13.5]: Right hand operand may have persistent side effect, Location: %s",
          location);
      analyzer::proto::Result* pb_result = AddResultToResultsList(
          results_list_, path, line_number, error_message);
      pb_result->set_error_kind(
          analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_13_5);
      pb_result->set_loc(location);
      LOG(INFO) << error_message;
    }
  }

 private:
  ResultsList* results_list_;
};

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  castCallback_ = new CastCallback;
  castCallback_->Init(results_list_, &finder_);
}

}  // namespace rule_13_5
}  // namespace misra
