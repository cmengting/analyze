/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_CWE_cwe_20_LIBTOOLING_CHECKER_H_
#define ANALYZER_CWE_cwe_20_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace autosar {
namespace rule_A2_7_3 {
namespace libtooling {
class Callback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  Callback* callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace rule_A2_7_3
}  // namespace autosar

#endif  // ANALYZER_CWE_cwe_20_LIBTOOLING_CHECKER_H_
