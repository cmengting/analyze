/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1167/libtooling/checker.h"

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
      "Avoid virtual method calls in constructors, and avoid initialization that can fail if you can't signal an error";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1167 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        cxxConstructorDecl(
            hasBody(compoundStmt(forEachDescendant(callExpr().bind("call")))))
            .bind("ctor"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    if (const auto* call = result.Nodes.getNodeAs<CallExpr>("call")) {
      if (misra::libtooling_utils::IsInSystemHeader(call, result.Context))
        return;

      // Just whether it is a virtual method call
      if (const CXXMethodDecl* method =
              dyn_cast<CXXMethodDecl>(call->getCalleeDecl())) {
        if (!method->isVirtual()) return;

        ReportError(
            misra::libtooling_utils::GetFilename(call, result.SourceManager),
            misra::libtooling_utils::GetLine(call, result.SourceManager),
            results_list_);
      }
    }
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
}  // namespace g1167
}  // namespace googlecpp
