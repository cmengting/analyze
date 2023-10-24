/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1185/libtooling/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message =
      "Do not put large method definitions inline in the class definition";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1185 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(cxxMethodDecl(unless(isExpansionInSystemHeader()),
                                     anyOf(isInline(), isConstexpr()))
                           .bind("func"),
                       this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const auto* func = result.Nodes.getNodeAs<FunctionDecl>("func");
    // Get the number of lines of a function
    auto* SM = result.SourceManager;
    SourceRange funcRange = func->getSourceRange();
    unsigned funcBeginLine = SM->getSpellingLineNumber(funcRange.getBegin());
    unsigned funcEndLine = SM->getSpellingLineNumber(funcRange.getEnd());
    unsigned funcLineCount = funcEndLine - funcBeginLine + 1;
    if (funcLineCount <= 10) return;
    ReportError(misra::libtooling_utils::GetFilename(func, SM),
                misra::libtooling_utils::GetLine(func, SM), results_list_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}
}  // namespace libtooling
}  // namespace g1185
}  // namespace googlecpp
