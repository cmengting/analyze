/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A5_1_2/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "Variables shall not be implicitly captured in a lambda expression.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A5_1_2 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(
      lambdaExpr(unless(isExpansionInSystemHeader())).bind("lambda"), this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const LambdaExpr* lambda = result.Nodes.getNodeAs<LambdaExpr>("lambda");
  for (auto it = lambda->capture_begin(); it != lambda->capture_end(); ++it) {
    if (it->isImplicit()) {
      ReportError(
          misra::libtooling_utils::GetFilename(lambda, result.SourceManager),
          misra::libtooling_utils::GetLine(lambda, result.SourceManager),
          results_list_);
      return;
    }
  }
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A5_1_2
}  // namespace autosar
