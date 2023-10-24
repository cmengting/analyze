/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A6_5_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "For-init-statement and expression should not perform actions other than loop-counter initialization and modification.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A6_5_4 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  auto loop_counter_matcher =
      CreateLoopCounterMatcher(misra::libtooling_utils::AllCondFormat,
                               misra::libtooling_utils::AllIncFormat);
  auto loop_counter_vardecl = varDecl(equalsBoundNode("loop_counter"));
  finder->addMatcher(
      forStmt(unless(isExpansionInSystemHeader()), loop_counter_matcher,
              hasLoopInit(unless(anyOf(
                  stmt(declStmt(declCountIs(1),
                                has(varDecl(equalsBoundNode("loop_counter"))))),
                  binaryOperator(isAssignmentOperator(),
                                 hasLHS(declRefExpr(to(loop_counter_vardecl)))),
                  unaryOperator(has(declRefExpr(to(loop_counter_vardecl))))))))
          .bind("for_stmt"),
      this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const ForStmt* for_stmt = result.Nodes.getNodeAs<ForStmt>("for_stmt");
  ReportError(
      misra::libtooling_utils::GetFilename(for_stmt, result.SourceManager),
      misra::libtooling_utils::GetLine(for_stmt, result.SourceManager),
      results_list_);
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A6_5_4
}  // namespace autosar
