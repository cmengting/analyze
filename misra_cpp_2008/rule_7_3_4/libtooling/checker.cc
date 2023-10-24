/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

// This rule only focus on one use, so it's not a dead store problem

#include "misra_cpp_2008/rule_7_3_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list_) {
  std::string error_message = absl::StrFormat("不得使用using指令");
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list_, path, line_number, error_message);
}

}  // namespace

namespace misra_cpp_2008 {
namespace rule_7_3_4 {
namespace libtooling {

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  finder_.addMatcher(usingDirectiveDecl().bind("using"), this);
}

void Checker::run(const MatchFinder::MatchResult& result) {
  const UsingDirectiveDecl* using_ =
      result.Nodes.getNodeAs<UsingDirectiveDecl>("using");
  std::string path_ =
      misra::libtooling_utils::GetFilename(using_, result.SourceManager);
  int line_number_ =
      misra::libtooling_utils::GetLine(using_, result.SourceManager);
  const SourceLocation loc = using_->getLocation();
  if (loc.isInvalid() ||
      result.Context->getSourceManager().isInSystemHeader(loc)) {
    return;
  }
  ReportError(path_, line_number_, results_list_);
}

}  // namespace libtooling
}  // namespace rule_7_3_4
}  // namespace misra_cpp_2008
