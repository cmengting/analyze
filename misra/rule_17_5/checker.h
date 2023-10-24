/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_17_5_CHECKER_H_
#define ANALYZER_MISRA_RULE_17_5_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_17_5 {

class CallExprCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* result_list);
  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  CallExprCallback* callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_17_5
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_17_5_CHECKER_H_
