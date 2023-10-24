/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A12_8_7/libtooling/checker.h"

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
      "Assignment operators should be declared with the ref-qualifier.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A12_8_7 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    finder->addMatcher(
        cxxMethodDecl(isUserProvided(), unless(isExpansionInSystemHeader()))
            .bind("decl"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const CXXMethodDecl* decl = result.Nodes.getNodeAs<CXXMethodDecl>("decl");
    OverloadedOperatorKind op = decl->getOverloadedOperator();
    set<OverloadedOperatorKind> assignment_operators{OO_Equal,
                                                     OO_PlusEqual,
                                                     OO_MinusEqual,
                                                     OO_StarEqual,
                                                     OO_SlashEqual,
                                                     OO_PercentEqual,
                                                     OO_CaretEqual,
                                                     OO_AmpEqual,
                                                     OO_PipeEqual,
                                                     OO_LessLessEqual,
                                                     OO_GreaterGreaterEqual,
                                                     OO_EqualEqual,
                                                     OO_ExclaimEqual,
                                                     OO_LessEqual,
                                                     OO_GreaterEqual};

    if (op != clang::OO_None && decl->getRefQualifier() == RQ_None) {
      if (assignment_operators.find(op) != assignment_operators.end()) {
        std::string path =
            misra::libtooling_utils::GetFilename(decl, result.SourceManager);
        int line_number =
            misra::libtooling_utils::GetLine(decl, result.SourceManager);
        ReportError(path, line_number, results_list_);
        return;
      }
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
}  // namespace rule_A12_8_7
}  // namespace autosar
