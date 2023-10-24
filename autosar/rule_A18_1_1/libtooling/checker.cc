/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A18_1_1/libtooling/checker.h"

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
  std::string error_message = "C-style arrays shall not be used.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A18_1_1 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        varDecl(hasType(arrayType()), unless(isExpansionInSystemHeader()))
            .bind("c_array_vd"),
        this);
    finder->addMatcher(
        fieldDecl(hasType(arrayType()), unless(isExpansionInSystemHeader()))
            .bind("c_array"),
        this);
    finder->addMatcher(
        functionDecl(returns(arrayType()), unless(isExpansionInSystemHeader()))
            .bind("c_array"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const Decl* d = result.Nodes.getNodeAs<Decl>("c_array");
    const VarDecl* vd = result.Nodes.getNodeAs<VarDecl>("c_array_vd");
    if (d) {
      ReportError(misra::libtooling_utils::GetFilename(d, result.SourceManager),
                  misra::libtooling_utils::GetLine(d, result.SourceManager),
                  results_list_);
    }
    if (vd && !(vd->isStaticDataMember() && vd->isConstexpr())) {
      ReportError(
          misra::libtooling_utils::GetFilename(vd, result.SourceManager),
          misra::libtooling_utils::GetLine(vd, result.SourceManager),
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
}  // namespace rule_A18_1_1
}  // namespace autosar
