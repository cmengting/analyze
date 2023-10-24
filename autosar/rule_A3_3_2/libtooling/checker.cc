/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A3_3_2/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "Static and thread-local objects shall be constant-initialized.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A3_3_2 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(varDecl(unless(isExpansionInSystemHeader())).bind("var"),
                     this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const VarDecl* var = result.Nodes.getNodeAs<VarDecl>("var");
  if ((!var->hasGlobalStorage() && !var->hasExternalStorage()) ||
      var->hasConstantInitialization() || !var->hasInit())
    return;
  std::string path =
      misra::libtooling_utils::GetFilename(var, result.SourceManager);
  int line_number = misra::libtooling_utils::GetLine(var, result.SourceManager);
  ReportError(path, line_number, results_list_);
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A3_3_2
}  // namespace autosar
