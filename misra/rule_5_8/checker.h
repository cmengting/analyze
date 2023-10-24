/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_5_8_CHECKER_H_
#define ANALYZER_MISRA_RULE_5_8_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>

#include "misra/proto_util.h"

namespace misra {
namespace rule_5_8 {

class ExternalVDCallback;
class ExternalFDCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  std::unordered_map<std::string, std::string> external_name_locations_;
  std::unordered_map<std::string, std::string> non_external_name_locations_;
  ExternalVDCallback* external_vd_callback_;
  ExternalFDCallback* external_fd_callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_5_8
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_5_8_CHECKER_H_
