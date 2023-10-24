/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A10_3_5/libtooling/checker.h"

#include <glog/logging.h>

#include <unordered_set>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

constexpr char kCXXMethodDeclString[] = "cxxMethodDecl";
const std::unordered_set<OverloadedOperatorKind> kAssignOpsSet = {
    OO_AmpEqual,      OO_CaretEqual,   OO_Equal,     OO_MinusEqual,
    OO_PercentEqual,  OO_SlashEqual,   OO_StarEqual, OO_GreaterGreaterEqual,
    OO_LessLessEqual, OO_ExclaimEqual, OO_PipeEqual, OO_PlusEqual};

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "A user-defined assignment operator shall not be virtual.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A10_3_5 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(cxxMethodDecl(isVirtual()).bind(kCXXMethodDeclString),
                     this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const CXXMethodDecl* cxx_method_decl =
      result.Nodes.getNodeAs<CXXMethodDecl>(kCXXMethodDeclString);
  if (cxx_method_decl->isOverloadedOperator() &&
      kAssignOpsSet.find(cxx_method_decl->getOverloadedOperator()) !=
          kAssignOpsSet.end()) {
    std::string path = misra::libtooling_utils::GetFilename(
        cxx_method_decl, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(cxx_method_decl, result.SourceManager);
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
}  // namespace rule_A10_3_5
}  // namespace autosar
