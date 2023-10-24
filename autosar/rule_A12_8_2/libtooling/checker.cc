/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A12_8_2/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "User-defined copy and move assignment operators should use user-defined no-throw swap function.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A12_8_2 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        cxxMethodDecl(
            anyOf(isCopyAssignmentOperator(), isMoveAssignmentOperator()),
            unless(hasDescendant(callExpr(
                callee(functionDecl(unless(hasDescendant(cxxThrowExpr())),
                                    unless(isExpansionInSystemHeader())))))),
            unless(isExpansionInSystemHeader()))
            .bind("cmd"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const CXXMethodDecl* cmd = result.Nodes.getNodeAs<CXXMethodDecl>("cmd");
    if (cmd)
      ReportError(
          misra::libtooling_utils::GetFilename(cmd, result.SourceManager),
          misra::libtooling_utils::GetLine(cmd, result.SourceManager),
          results_list_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  results_list_ = result_list;
  callback_ = new Callback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A12_8_2
}  // namespace autosar
