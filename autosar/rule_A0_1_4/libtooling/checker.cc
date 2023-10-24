/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A0_1_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

constexpr char kNonVirtualFuncString[] = "nonVirtualFunc";

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "There shall be no unused named parameters in non-virtual functions.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A0_1_4 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(functionDecl(unless(cxxMethodDecl(isVirtual())))
                         .bind(kNonVirtualFuncString),
                     this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const FunctionDecl* function_decl =
      result.Nodes.getNodeAs<FunctionDecl>(kNonVirtualFuncString);
  if (function_decl == nullptr || !function_decl->hasBody() ||
      !function_decl->isFirstDecl()) {
    return;
  }
  unsigned int param_num = function_decl->getNumParams();
  for (unsigned int i = 0; i < param_num; ++i) {
    const ParmVarDecl* param_decl = function_decl->getParamDecl(i);
    if (!param_decl->getQualifiedNameAsString().empty() &&
        !param_decl->hasAttr<UnusedAttr>() &&
        !param_decl->isThisDeclarationReferenced()) {
      std::string path = misra::libtooling_utils::GetFilename(
          param_decl, result.SourceManager);
      int line_number =
          misra::libtooling_utils::GetLine(param_decl, result.SourceManager);
      ReportError(path, line_number, results_list_);
    }
  }
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A0_1_4
}  // namespace autosar
