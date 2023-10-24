/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A12_8_4/libtooling/checker.h"

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
      "Move constructor shall not initialize its class members and base classes using copy semantics.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A12_8_4 {
namespace libtooling {
class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    auto scalar_type =
        anyOf(hasType(pointerType()), hasType(builtinType()),
              hasType(enumType()), hasType(hasCanonicalType(builtinType())));

    auto no_movector_matcher =
        declRefExpr(
            hasAncestor(cxxConstructorDecl(
                isMoveConstructor(),
                forEachConstructorInitializer(cxxCtorInitializer(
                    forField(unless(scalar_type)), isMemberInitializer(),
                    withInitializer(unless(hasDescendant(callExpr(
                        callee(functionDecl(hasName("std::move"))))))))))),
            hasAncestor(memberExpr()))
            .bind("decl");
    finder->addMatcher(no_movector_matcher, this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const DeclRefExpr* decl = result.Nodes.getNodeAs<DeclRefExpr>("decl");
    const SourceLocation loc = decl->getLocation();
    if (!result.Context->getSourceManager().isInSystemHeader(loc)) {
      ReportError(
          misra::libtooling_utils::GetFilename(decl, result.SourceManager),
          misra::libtooling_utils::GetLine(decl, result.SourceManager),
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
}  // namespace rule_A12_8_4
}  // namespace autosar
