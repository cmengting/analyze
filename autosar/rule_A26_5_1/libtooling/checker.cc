/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A26_5_1/libtooling/checker.h"

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
      "Pseudorandom numbers shall not be generated using std::rand().";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A26_5_1 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        declRefExpr(hasDeclaration(
                        functionDecl(isExpansionInSystemHeader()).bind("fd")),
                    unless(isExpansionInSystemHeader()))
            .bind("dre"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const DeclRefExpr* dre = result.Nodes.getNodeAs<DeclRefExpr>("dre");
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    if (dre && fd) {
      string fd_name = fd->getQualifiedNameAsString();
      bool is_rand = (fd_name == "rand") || (fd_name == "std::shuffle") ||
                     (fd_name == "std::random_shuffle");
      if (is_rand)
        ReportError(
            misra::libtooling_utils::GetFilename(dre, result.SourceManager),
            misra::libtooling_utils::GetLine(dre, result.SourceManager),
            results_list_);
    }
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
}  // namespace rule_A26_5_1
}  // namespace autosar
