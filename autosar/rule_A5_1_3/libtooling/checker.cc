/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A5_1_3/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;
using analyzer::proto::ResultsList;

namespace {
constexpr char kLambdaExprString[] = "lambdaExpr";
void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "Parameter list (possibly empty) shall be included in every lambda expression.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A5_1_3 {
namespace libtooling {
void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(lambdaExpr().bind(kLambdaExprString), this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const LambdaExpr* lambda_expr =
      result.Nodes.getNodeAs<LambdaExpr>(kLambdaExprString);
  if (!lambda_expr->hasExplicitParameters()) {
    std::string path =
        misra::libtooling_utils::GetFilename(lambda_expr, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(lambda_expr, result.SourceManager);
    ReportError(path, line_number, results_list_);
  }
}

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_A5_1_3
}  // namespace autosar
