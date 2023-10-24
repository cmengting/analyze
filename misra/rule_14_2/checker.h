/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_14_2_CHECKER_H_
#define ANALYZER_MISRA_RULE_14_2_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_14_2 {
class FirstDefCallback;
class FirstNotDefCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  FirstDefCallback* def_callback_;
  FirstNotDefCallback* nodef_callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_14_2
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_14_2_CHECKER_H_
