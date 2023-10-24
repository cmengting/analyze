/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_5_2_6/libtooling/checker.h"

#include <glog/logging.h>

#include <climits>
#include <unordered_map>
#include <vector>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string loc, int line_number, ResultsList* results_list) {
  std::string error_message =
      "显式类型转换不得将一个函数指针转换为任何其他的指针类型，包括函数指针类型";
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, loc, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_5_2_6);
}
}  // namespace

namespace misra_cpp_2008 {
namespace rule_5_2_6 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(castExpr(hasSourceExpression(hasType(pointerType(
                                    pointee(ignoringParens(functionType()))))))
                           .bind("typeFromFuncPointer"),
                       this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const CastExpr* ce =
        result.Nodes.getNodeAs<CastExpr>("typeFromFuncPointer");

    std::string path =
        misra::libtooling_utils::GetFilename(ce, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(ce, result.SourceManager);
    ReportError(path, line_number, results_list_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_5_2_6
}  // namespace misra_cpp_2008
