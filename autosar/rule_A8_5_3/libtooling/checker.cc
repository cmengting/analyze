/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A8_5_3/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace autosar {
namespace rule_A8_5_3 {
namespace libtooling {

void ReportError(string path, int line_number,
                 analyzer::proto::ResultsList* results_list) {
  string error_message =
      "A variable of type auto shall not be initialized using {} or ={} braced-initialization.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        varDecl(hasInitializer(initListExpr()), hasType(autoType()))
            .bind("bracedInit"),
        this);
    finder->addMatcher(
        varDecl(hasType(autoType()), hasDescendant(cxxStdInitializerListExpr()))
            .bind("bracedInitStmt"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const VarDecl* varDecl = result.Nodes.getNodeAs<VarDecl>("bracedInit");

    if (varDecl &&
        !misra::libtooling_utils::IsInSystemHeader(varDecl, result.Context)) {
      ReportError(
          misra::libtooling_utils::GetFilename(varDecl, result.SourceManager),
          misra::libtooling_utils::GetLine(varDecl, result.SourceManager),
          results_list_);
    }

    const auto* declStmt = result.Nodes.getNodeAs<VarDecl>("bracedInitStmt");
    if (declStmt &&
        !misra::libtooling_utils::IsInSystemHeader(declStmt, result.Context)) {
      ReportError(
          misra::libtooling_utils::GetFilename(declStmt, result.SourceManager),
          misra::libtooling_utils::GetLine(declStmt, result.SourceManager),
          results_list_);
    }
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A8_5_3
}  // namespace autosar