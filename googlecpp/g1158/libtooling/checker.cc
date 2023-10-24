/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1158/libtooling/checker.h"

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
      "Do not use using-directives (e.g., using namespace foo)";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1158 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(
      usingDirectiveDecl(unless(isExpansionInSystemHeader())).bind("use"),
      this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const UsingDirectiveDecl* target =
      result.Nodes.getNodeAs<UsingDirectiveDecl>("use");
  // Sema creates an implicit UsingDirective for namespace {}, we ignore it
  if (target->getNominatedNamespace()->isAnonymousNamespace()) return;
  SourceLocation loc = target->getBeginLoc();
  ReportError(
      misra::libtooling_utils::GetLocationFilename(loc, result.SourceManager),
      misra::libtooling_utils::GetLocationLine(loc, result.SourceManager),
      results_list_);
}

}  // namespace libtooling
}  // namespace g1158
}  // namespace googlecpp
