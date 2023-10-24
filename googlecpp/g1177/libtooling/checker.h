/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1177_LIBTOOLING_LIB_H_
#define ANALYZER_GOOGLECPP_G1177_LIBTOOLING_LIB_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1177 {
namespace libtooling {
class CastCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  CastCallback* callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace g1177
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1177_LIBTOOLING_LIB_H_
