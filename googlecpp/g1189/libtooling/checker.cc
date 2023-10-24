/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1189/libtooling/checker.h"

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
      "If a function exceeds about 40 lines, think about whether it can be broken up without harming the structure of the program";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1189 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder, int maximum_allowed_func_line) {
    results_list_ = results_list;
    finder->addMatcher(functionDecl().bind("func"), this);
    maximum_allowed_func_line_ = maximum_allowed_func_line;
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    if (const auto* func = result.Nodes.getNodeAs<FunctionDecl>("func")) {
      if (misra::libtooling_utils::IsInSystemHeader(func, result.Context))
        return;

      // Get the number of lines of a function
      auto* SM = result.SourceManager;
      SourceRange funcRange = func->getSourceRange();
      // getExpansionLineNumber will return the correct line number after
      // macro expansion
      unsigned funcBeginLine = SM->getExpansionLineNumber(funcRange.getBegin());
      unsigned funcEndLine = SM->getExpansionLineNumber(funcRange.getEnd());
      unsigned funcLineCount = funcEndLine - funcBeginLine + 1;
      if (funcLineCount <= maximum_allowed_func_line_) return;

      ReportError(misra::libtooling_utils::GetFilename(func, SM),
                  misra::libtooling_utils::GetLine(func, SM), results_list_);
    }
  }

 private:
  analyzer::proto::ResultsList* results_list_;
  int maximum_allowed_func_line_;
};

void Checker::Init(analyzer::proto::ResultsList* results_list,
                   int maximum_allowed_func_line) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_, maximum_allowed_func_line);
}
}  // namespace libtooling
}  // namespace g1189
}  // namespace googlecpp
