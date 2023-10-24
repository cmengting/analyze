/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A8_4_9/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;
using namespace misra::libtooling_utils;

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "\"in-out\" parameters declared as T & shall be modified.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

namespace autosar {
namespace rule_A8_4_9 {
namespace libtooling {

extern FuncInfo2ParamInfos func_info_2_param_infos;

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  AddFuncOutputParamMatchers(finder, this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  FuncOutputParamCallback(result, func_info_2_param_infos);
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A8_4_9
}  // namespace autosar
