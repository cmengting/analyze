/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A4_7_1/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang::ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "An integer expression shall not lead to data loss.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A4_7_1 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(cxxStaticCastExpr().bind("cast"), this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const CastExpr* castNode = result.Nodes.getNodeAs<CastExpr>("cast");
  if (misra::libtooling_utils::IsInSystemHeader(castNode, result.Context)) {
    return;
  }
  QualType originalType = castNode->getSubExprAsWritten()->getType();
  QualType targetType = castNode->getType();
  if (originalType->isIntegerType() && targetType->isIntegerType()) {
    unsigned origBitWidth = result.Context->getTypeSize(originalType);
    unsigned targetBitWidth = result.Context->getTypeSize(targetType);
    if (origBitWidth > targetBitWidth) {
      std::string path =
          misra::libtooling_utils::GetFilename(castNode, result.SourceManager);
      int line_number =
          misra::libtooling_utils::GetLine(castNode, result.SourceManager);
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
}  // namespace rule_A4_7_1
}  // namespace autosar
