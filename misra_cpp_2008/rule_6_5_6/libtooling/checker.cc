/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_6_5_6/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::libtooling_utils;
using namespace llvm;

namespace misra_cpp_2008 {
namespace rule_6_5_6 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    auto loop_counter_matcher =
        CreateLoopCounterMatcher(AllCondFormat, AllIncFormat);

    finder->addMatcher(
        forStmt(loop_counter_matcher,
                hasCondition(forEachDescendant(declRefExpr(unless(
                    anyOf(hasType(booleanType()),
                          to(varDecl(equalsBoundNode("loop_counter")))))))))
            .bind("forStmt"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const ForStmt* s = result.Nodes.getNodeAs<ForStmt>("forStmt");

    if (misra::libtooling_utils::IsInSystemHeader(s, result.Context)) {
      return;
    }
    string error_message =
        "除了在语句中被修改的循环计数器外，其他的循环控制变量应具有bool类型";
    string path = misra::libtooling_utils::GetFilename(s, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(s, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_6_5_6);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_6_5_6
}  // namespace misra_cpp_2008
