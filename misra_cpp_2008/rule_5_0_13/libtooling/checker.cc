/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_5_0_13/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace misra_cpp_2008 {
namespace rule_5_0_13 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    auto impcast =
        implicitCastExpr(unless(hasSourceExpression(hasType(booleanType()))),
                         hasImplicitDestinationType(booleanType()));
    finder->addMatcher(
        ifStmt(hasCondition(
                   anyOf(impcast, binaryOperator(hasEitherOperand(impcast)))),
               unless(hasConditionVariableStatement(stmt())))
            .bind("stmt"),
        this);
    finder->addMatcher(
        whileStmt(hasCondition(anyOf(
                      impcast, binaryOperator(hasEitherOperand(impcast)))),
                  unless(hasDescendant(declStmt())))
            .bind("stmt"),
        this);
    finder->addMatcher(
        forStmt(hasCondition(
                    anyOf(impcast, binaryOperator(hasEitherOperand(impcast)))))
            .bind("stmt"),
        this);
    finder->addMatcher(
        doStmt(hasCondition(
                   anyOf(impcast, binaryOperator(hasEitherOperand(impcast)))))
            .bind("stmt"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Stmt* s = result.Nodes.getNodeAs<Stmt>("stmt");
    if (misra::libtooling_utils::IsInSystemHeader(s, result.Context)) {
      return;
    }
    string error_message = "if语句的条件和迭代语句的条件必须具有bool类型";
    string path = misra::libtooling_utils::GetFilename(s, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(s, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_5_0_13);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_5_0_13
}  // namespace misra_cpp_2008
