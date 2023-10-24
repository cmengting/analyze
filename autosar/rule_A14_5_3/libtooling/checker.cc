/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A14_5_3/libtooling/checker.h"

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
      "A non-member generic operator shall only be declared in a namespace that does not contain class (struct) type, enum type or union type declarations.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A14_5_3 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        functionTemplateDecl(hasParent(namespaceDecl(has(tagDecl()))),
                             unless(isExpansionInSystemHeader()))
            .bind("ftd"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const FunctionTemplateDecl* ftd =
        result.Nodes.getNodeAs<FunctionTemplateDecl>("ftd");
    if (ftd) {
      const FunctionDecl* fd = ftd->getTemplatedDecl();
      if (fd->isOverloadedOperator())
        ReportError(
            misra::libtooling_utils::GetFilename(fd, result.SourceManager),
            misra::libtooling_utils::GetLine(fd, result.SourceManager),
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
}  // namespace rule_A14_5_3
}  // namespace autosar
