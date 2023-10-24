/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1189_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1189_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1189 {
namespace libtooling {
class Callback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            int maximum_allowed_func_line);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  Callback* callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace g1189
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1189_LIBTOOLING_CHECKER_H_
