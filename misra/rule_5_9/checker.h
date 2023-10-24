/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_5_9_CHECKER_H_
#define ANALYZER_MISRA_RULE_5_9_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>

#include "misra/proto_util.h"

namespace misra {
namespace rule_5_9 {

class InternalVDCallback;
class InternalFDCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  std::unordered_map<std::string, std::string> internal_name_locations_;
  std::unordered_map<std::string, std::string> non_internal_name_locations_;
  InternalVDCallback* internal_vd_callback_;
  InternalFDCallback* internal_fd_callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_5_9
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_5_9_CHECKER_H_
