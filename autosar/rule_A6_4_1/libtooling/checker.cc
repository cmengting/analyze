/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A6_4_1/libtooling/checker.h"

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
      "A switch statement shall have at least two case-clauses, distinct from the default label.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A6_4_1 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    finder->addMatcher(
        switchStmt(unless(isExpansionInSystemHeader())).bind("stmt"), this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const SwitchStmt* stmt = result.Nodes.getNodeAs<SwitchStmt>("stmt");
    const SwitchCase* p = stmt->getSwitchCaseList();
    int cnt = 0;
    while (p) {
      if (isa<CaseStmt>(p)) {
        cnt++;
      }
      if (cnt >= 2) {
        return;
      }
      p = p->getNextSwitchCase();
    }
    std::string path =
        misra::libtooling_utils::GetFilename(stmt, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(stmt, result.SourceManager);
    ReportError(path, line_number, results_list_);
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
}  // namespace rule_A6_4_1
}  // namespace autosar
