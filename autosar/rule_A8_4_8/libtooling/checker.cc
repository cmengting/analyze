/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A8_4_8/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message = "Output parameters shall not be used.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A8_4_8 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(
      functionDecl(unless(isExpansionInSystemHeader())).bind("func"), this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const FunctionDecl* func = result.Nodes.getNodeAs<FunctionDecl>("func");
  if (func->isImplicit()) return;
  if (func->getNumParams() <= func->isVariadic()) return;
  for (unsigned int i = 0; i < func->getNumParams() - func->isVariadic(); ++i) {
    ParmVarDecl const* const param = func->getParamDecl(i);
    QualType const& type = param->getType();
    if (type.isNull()) continue;
    if ((type->isReferenceType() || type->isPointerType()) &&
        !type.getNonReferenceType().isConstQualified()) {
      std::string path =
          misra::libtooling_utils::GetFilename(func, result.SourceManager);
      int line_number =
          misra::libtooling_utils::GetLine(func, result.SourceManager);
      ReportError(path, line_number, results_list_);
      break;
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
}  // namespace rule_A8_4_8
}  // namespace autosar
