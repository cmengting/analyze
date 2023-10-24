/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_2_3_CHECKER_H_
#define ANALYZER_MISRA_RULE_2_3_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

namespace misra {
namespace rule_2_3 {

using MatchFinder = clang::ast_matchers::MatchFinder;

class Checker : public MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  MatchFinder* GetMatchFinder() { return &finder_; }

  void run(const MatchFinder::MatchResult& result) override;

 private:
  MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_2_3
}  // namespace misra

#endif  // MISRA_RULE_2_3_CHECKER_H_
