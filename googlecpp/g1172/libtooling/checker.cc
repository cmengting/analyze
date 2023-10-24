/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1172/libtooling/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message = "(struct) All fields must be public";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1172 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    auto nonPublicMatcher = anyOf(isPrivate(), isProtected());
    auto nonPublicFieldMatcher = recordDecl(
        isStruct(), forEach(fieldDecl(nonPublicMatcher).bind("nonpublic")));
    auto nonPublicMethodMatcher = recordDecl(
        isStruct(), forEach(cxxMethodDecl(nonPublicMatcher).bind("nonpublic")));
    auto nonPublicTypeMatcher = recordDecl(
        isStruct(),
        forEach(typedefNameDecl(nonPublicMatcher).bind("nonpublic")));
    auto nonPublicTemplateMatcher = recordDecl(
        isStruct(),
        forEach(functionTemplateDecl(nonPublicMatcher).bind("nonpublic")));
    auto nonPublicEnumMatcher = recordDecl(
        isStruct(), forEach(enumDecl(nonPublicMatcher).bind("nonpublic")));
    auto nonPublicRecordMatcher = recordDecl(
        isStruct(), forEach(recordDecl(nonPublicMatcher).bind("nonpublic")));
    finder->addMatcher(nonPublicFieldMatcher, this);
    finder->addMatcher(nonPublicMethodMatcher, this);
    finder->addMatcher(nonPublicTypeMatcher, this);
    finder->addMatcher(nonPublicTemplateMatcher, this);
    finder->addMatcher(nonPublicEnumMatcher, this);
    finder->addMatcher(nonPublicRecordMatcher, this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    if (const auto* nonPublic = result.Nodes.getNodeAs<Decl>("nonpublic")) {
      if (misra::libtooling_utils::IsInSystemHeader(nonPublic, result.Context))
        return;
      ReportError(
          misra::libtooling_utils::GetFilename(nonPublic, result.SourceManager),
          misra::libtooling_utils::GetLine(nonPublic, result.SourceManager),
          results_list_);
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
}  // namespace g1172
}  // namespace googlecpp
