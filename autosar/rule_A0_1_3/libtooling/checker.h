/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A0_1_3_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A0_1_3_LIBTOOLING_CHECKER_H_

#include <string>
#include <unordered_map>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;
using MatchFinder = clang::ast_matchers::MatchFinder;

using analyzer::proto::ResultsList;

namespace autosar {
namespace rule_A0_1_3 {
namespace libtooling {
class Callback;

class Checker {
 public:
  void Init(ResultsList* results_list);
  MatchFinder* GetMatchFinder();

 private:
  Callback* callback_;
  MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_A0_1_3
}  // namespace autosar

#endif
