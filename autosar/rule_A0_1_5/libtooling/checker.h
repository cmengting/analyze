/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A0_1_5_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A0_1_5_LIBTOOLING_CHECKER_H_

#include <unordered_map>
#include <vector>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;
using MatchFinder = clang::ast_matchers::MatchFinder;

using analyzer::proto::ResultsList;

namespace {

// This is used to store some basic information of a function.
struct VirtualFuncInfo {
  std::string path;
  int line_number;
  std::vector<bool> params_used_info_;
  std::vector<std::string> overridden_method_names_;
};
}  // namespace

namespace autosar {
namespace rule_A0_1_5 {
namespace libtooling {
class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder);
  void run(const MatchFinder::MatchResult& result);
  void MarkFuncParamsUsed(const std::string& func_decl_sig);
  void Report();

 private:
  ResultsList* results_list_;
  std::unordered_map<std::string, VirtualFuncInfo> funcs_info_;
};

class Checker {
 public:
  void Init(ResultsList* results_list);
  MatchFinder* GetMatchFinder();
  void Report();

 private:
  Callback* callback_;
  MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_A0_1_5
}  // namespace autosar

#endif
