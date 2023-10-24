/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1202/libtooling/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message = "Use nullptr for pointers, and '\\0' for chars";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1202 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    // To catch every use of NULL
    finder->addMatcher(gnuNullExpr().bind("NULL"), this);

    // To catch char c = 0;
    // We match each cast from 0 to char
    auto intLiteralZero = integerLiteral(equals(0));
    finder->addMatcher(
        castExpr(hasType(isAnyCharacter()), has(intLiteralZero)).bind("cast"),
        this);

    // If we allow `(int)'a' > 0`
    // then change the following castExpr to implicitCastExpr
    auto elevatedChar = castExpr(has(expr(hasType(isAnyCharacter()))));

    // To catch 'a' != 0
    finder->addMatcher(
        binaryOperator(
            isComparisonOperator(),  // we may allow arithmetic expression
                                     // like 'a'+0;
            anyOf(binaryOperator(hasLHS(elevatedChar), hasRHS(intLiteralZero))
                      .bind("BOP"),
                  binaryOperator(hasLHS(intLiteralZero), hasRHS(elevatedChar))
                      .bind("BOP"))),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    auto Report = [&](const Stmt* stmt) {
      if (misra::libtooling_utils::IsInSystemHeader(stmt, result.Context))
        return;
      ReportError(
          misra::libtooling_utils::GetFilename(stmt, result.SourceManager),
          misra::libtooling_utils::GetLine(stmt, result.SourceManager),
          results_list_);
    };

    if (const auto* cast = result.Nodes.getNodeAs<CastExpr>("cast"))
      Report(cast);
    if (const auto* GNUNull = result.Nodes.getNodeAs<GNUNullExpr>("NULL"))
      Report(GNUNull);
    if (const auto* bop = result.Nodes.getNodeAs<BinaryOperator>("BOP"))
      Report(bop);
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
}  // namespace g1202
}  // namespace googlecpp
