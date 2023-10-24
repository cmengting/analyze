/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_5_0_14/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace llvm;
using namespace clang::ast_matchers;

namespace misra_cpp_2008 {
namespace rule_5_0_14 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        conditionalOperator(hasCondition(
            expr(unless(hasType(booleanType()))).bind("condition"))),
        this);
    finder->addMatcher(conditionalOperator(hasCondition(
                           implicitCastExpr(hasSourceExpression(
                                                unless(hasType(booleanType()))))
                               .bind("condition"))),
                       this);
  }
  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Expr* condition = result.Nodes.getNodeAs<Expr>("condition");
    condition->dump();

    if (misra::libtooling_utils::IsInSystemHeader(condition, result.Context)) {
      return;
    }
    string error_message = "条件运算符的第一个操作数必须具有bool类型";
    string path =
        misra::libtooling_utils::GetFilename(condition, result.SourceManager);
    int line =
        misra::libtooling_utils::GetLine(condition, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_5_0_14);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};
void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}

}  // namespace libtooling
}  // namespace rule_5_0_14
}  // namespace misra_cpp_2008
