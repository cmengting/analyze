/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A5_1_1/libtooling/checker.h"

#include <glog/logging.h>

#include <algorithm>
#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message =
      "Literal values shall not be used apart from type initialization, otherwise symbolic names shall be used instead.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A5_1_1 {
namespace libtooling {

const auto parentMatcher = anyOf(hasParent(translationUnitDecl()),
                                 hasParent(stmt()), hasParent(decl()));
decltype(parentMatcher) nestedHasParentMatcher(int level) {
  if (level == 1) return parentMatcher;
  auto matcher = nestedHasParentMatcher(level - 1);
  return anyOf(hasParent(translationUnitDecl()), hasParent(stmt(matcher)),
               hasParent(decl(matcher)));
}

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    const auto matcher = allOf(
        // skip 5 in `int x[5];`, `C<int, 5> c;`, `std::array<int, 5>` as they
        // are orphan nodes without more than 3 levels of parent
        nestedHasParentMatcher(3),
        unless(hasParent(varDecl(hasInitializer(expr())))),
        unless(hasParent(cxxConstructorDecl())),
        unless(hasAncestor(initListExpr())),
        unless(hasParent(cxxConstructExpr())),
        unless(hasParent(varDecl(isTemplateInstantiation()))),
        unless(hasAncestor(cxxThrowExpr())),
        unless(hasAncestor(cxxOperatorCallExpr(
            hasOverloadedOperatorName("<<")))  // match logging mechanism
               ),
        unless(isExpansionInSystemHeader()));
    finder->addMatcher(integerLiteral(matcher).bind("literal"), this);
    finder->addMatcher(stringLiteral(matcher).bind("literal"), this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const auto* literal = result.Nodes.getNodeAs<Expr>("literal");
    ReportError(
        misra::libtooling_utils::GetFilename(literal, result.SourceManager),
        misra::libtooling_utils::GetLine(literal, result.SourceManager),
        results_list_);
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
}  // namespace rule_A5_1_1
}  // namespace autosar
