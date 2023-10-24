/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_RULE_0_1_8_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_RULE_0_1_8_LIBTOOLING_CHECKER_H_

#include <string.h>

#include <unordered_map>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace misra_cpp_2008 {
namespace rule_0_1_8 {
namespace libtooling {

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

}  // namespace libtooling
}  // namespace rule_0_1_8
}  // namespace misra_cpp_2008

#endif  // ANALYZER_MISRA_CPP_2008_RULE_0_1_8_LIBTOOLING_CHECKER_H_
