/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1190/libtooling/checker.h"

#include <clang/Basic/Builtins.h>
#include <glog/logging.h>

#include <string>

#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message =
      "Default arguments are banned on virtual functions, where they don't work properly, and in cases where the specified default might not evaluate to the same value depending on when it was evaluated";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1190 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(
      cxxMethodDecl(unless(isExpansionInSystemHeader()), isVirtual(),
                    hasAnyParameter(parmVarDecl(hasInitializer(expr()))))
          .bind("decl"),
      this);
  finder->addMatcher(
      cxxMethodDecl(
          unless(isExpansionInSystemHeader()),
          hasAnyParameter(parmVarDecl(hasInitializer(expr().bind("exp")))))
          .bind("decl"),
      this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const CXXMethodDecl* decl = result.Nodes.getNodeAs<CXXMethodDecl>("decl");
  const Expr* exp = result.Nodes.getNodeAs<Expr>("exp");
  SourceLocation loc = decl->getBeginLoc();
  if (exp && !exp->HasSideEffects(*result.Context)) return;
  ReportError(
      misra::libtooling_utils::GetLocationFilename(loc, result.SourceManager),
      misra::libtooling_utils::GetLocationLine(loc, result.SourceManager),
      results_list_);
}

}  // namespace libtooling
}  // namespace g1190
}  // namespace googlecpp
