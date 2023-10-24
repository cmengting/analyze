/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1269_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1269_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1269 {
namespace libtooling {
class FriendInSameFileCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  FriendInSameFileCallback* callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace g1269
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1269_LIBTOOLING_CHECKER_H_
