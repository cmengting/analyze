/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_RULE_4_5_3_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_RULE_4_5_3_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace misra_cpp_2008 {
namespace rule_4_5_3 {
namespace libtooling {
class OPCallback;
class AddCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  OPCallback* op_callback_;
  AddCallback* add_callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_4_5_3
}  // namespace misra_cpp_2008

#endif  // ANALYZER_MISRA_CPP_2008_RULE_4_5_3_LIBTOOLING_CHECKER_H_
