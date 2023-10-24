/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1170/libtooling/checker.h"

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
      "To eliminate the risk of slicing, prefer to make base classes abstract, by making their constructors protected, by declaring their destructors protected, or by giving them one or more pure virtual member functions";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1170 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    auto pureClassMatcher =
        cxxRecordDecl(hasDefinition(), has(cxxMethodDecl(isPure())));
    auto protectedConstructorClassMatcher = cxxRecordDecl(
        hasDefinition(),
        forEach(cxxConstructorDecl(hasDeclContext(anything()), isProtected())));
    auto protectedDestructorClassMatcher = cxxRecordDecl(
        hasDefinition(),
        forEach(cxxDestructorDecl(hasDeclContext(anything()), isProtected())));
    auto concreteBaseClassMatcher = cxxRecordDecl(
        unless(pureClassMatcher), unless(protectedConstructorClassMatcher),
        unless(protectedDestructorClassMatcher));
    auto badDerivedClassMatcher =
        cxxRecordDecl(unless(isExpansionInSystemHeader()),
                      hasDirectBase(hasType(concreteBaseClassMatcher)))
            .bind("drivedclass");
    finder->addMatcher(badDerivedClassMatcher, this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    if (const auto* derived =
            result.Nodes.getNodeAs<CXXRecordDecl>("drivedclass")) {
      ReportError(
          misra::libtooling_utils::GetFilename(derived, result.SourceManager),
          misra::libtooling_utils::GetLine(derived, result.SourceManager),
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
}  // namespace g1170
}  // namespace googlecpp
