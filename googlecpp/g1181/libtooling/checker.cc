/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1181/libtooling/checker.h"

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
  string error_message = "Do not overload &&, ||, , (comma), or unary &";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1181 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(
      cxxMethodDecl(unless(isExpansionInSystemHeader()), parameterCountIs(0),
                    hasOverloadedOperatorName("&"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      cxxMethodDecl(unless(isExpansionInSystemHeader()), parameterCountIs(1),
                    hasOverloadedOperatorName("&&"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      cxxMethodDecl(unless(isExpansionInSystemHeader()), parameterCountIs(1),
                    hasOverloadedOperatorName("||"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      cxxMethodDecl(unless(isExpansionInSystemHeader()), parameterCountIs(1),
                    hasOverloadedOperatorName(","))
          .bind("ovl"),
      this);
  finder->addMatcher(
      functionDecl(unless(isExpansionInSystemHeader()), unless(cxxMethodDecl()),
                   parameterCountIs(1), hasOverloadedOperatorName("&"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      functionDecl(unless(isExpansionInSystemHeader()), unless(cxxMethodDecl()),
                   parameterCountIs(2), hasOverloadedOperatorName("&&"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      functionDecl(unless(isExpansionInSystemHeader()), unless(cxxMethodDecl()),
                   parameterCountIs(2), hasOverloadedOperatorName("||"))
          .bind("ovl"),
      this);
  finder->addMatcher(
      functionDecl(unless(isExpansionInSystemHeader()), unless(cxxMethodDecl()),
                   parameterCountIs(2), hasOverloadedOperatorName(","))
          .bind("ovl"),
      this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const FunctionDecl* target = result.Nodes.getNodeAs<FunctionDecl>("ovl");
  SourceLocation loc = target->getBeginLoc();
  ReportError(
      misra::libtooling_utils::GetLocationFilename(loc, result.SourceManager),
      misra::libtooling_utils::GetLocationLine(loc, result.SourceManager),
      results_list_);
}

}  // namespace libtooling
}  // namespace g1181
}  // namespace googlecpp
