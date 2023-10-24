/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_18_5_CHECKER_H_
#define ANALYZER_MISRA_RULE_18_5_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_18_5 {

class Callback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* result_list);
  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  Callback* callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_18_5
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_18_5_CHECKER_H_
