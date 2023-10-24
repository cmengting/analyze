/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_RULE_0_1_11_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_RULE_0_1_11_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace misra_cpp_2008 {
namespace rule_0_1_11 {
namespace libtooling {
class ParamCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  void Report();
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  ParamCallback* callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_0_1_11
}  // namespace misra_cpp_2008

#endif  // ANALYZER_MISRA_CPP_2008_RULE_4_5_2_LIBTOOLING_CHECKER_H_