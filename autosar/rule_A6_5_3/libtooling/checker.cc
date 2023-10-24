/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A6_5_3/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message = "Do statements should not be used.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A6_5_3 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(doStmt(unless(isExpansionInSystemHeader())).bind("do"),
                     this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const DoStmt* do_stmt = result.Nodes.getNodeAs<DoStmt>("do");
  if (misra::libtooling_utils::IsInMacroExpansion(do_stmt,
                                                  result.SourceManager)) {
    return;
  }
  ReportError(
      misra::libtooling_utils::GetFilename(do_stmt, result.SourceManager),
      misra::libtooling_utils::GetLine(do_stmt, result.SourceManager),
      results_list_);
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A6_5_3
}  // namespace autosar
