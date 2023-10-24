/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A13_2_3/libtooling/checker.h"

#include <glog/logging.h>

#include <unordered_set>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

const std::unordered_set<OverloadedOperatorKind> kRelationalOperatorSet = {
    OO_Less,         OO_Greater,   OO_EqualEqual,
    OO_ExclaimEqual, OO_LessEqual, OO_GreaterEqual};
constexpr char kOperatorOverloadFunctionDeclString[] =
    "operatorOverloadFunctionDecl";

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "A relational operator shall return a boolean value.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A13_2_3 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(functionDecl().bind(kOperatorOverloadFunctionDeclString),
                     this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const FunctionDecl* function_decl =
      result.Nodes.getNodeAs<FunctionDecl>(kOperatorOverloadFunctionDeclString);
  if (function_decl->isOverloadedOperator() &&
      kRelationalOperatorSet.find(function_decl->getOverloadedOperator()) !=
          kRelationalOperatorSet.end() &&
      ((isa<CXXMethodDecl>(function_decl) &&
        function_decl->getNumParams() == 1) ||
       (!isa<CXXMethodDecl>(function_decl) &&
        function_decl->getNumParams() == 2)) &&
      !function_decl->getReturnType()->isBooleanType()) {
    std::string path = misra::libtooling_utils::GetFilename(
        function_decl, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(function_decl, result.SourceManager);
    ReportError(path, line_number, results_list_);
  }
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A13_2_3
}  // namespace autosar
