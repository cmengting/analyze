/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_misra_cpp_2008_LIBTOOLING_RULE_8_4_4_CHECKER_H_
#define ANALYZER_misra_cpp_2008_LIBTOOLING_RULE_8_4_4_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using MatchFinder = clang::ast_matchers::MatchFinder;

namespace misra_cpp_2008 {
namespace rule_8_4_4 {
namespace libtooling {

class FuncIdentifierCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  MatchFinder finder_;
  FuncIdentifierCallback* callback_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace rule_8_4_4
}  // namespace misra_cpp_2008

#endif  // misra_cpp_2008_LIBTOOLING_RULE_8_4_4_CHECKER_H_
