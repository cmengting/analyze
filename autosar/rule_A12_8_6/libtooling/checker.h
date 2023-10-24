/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A12_8_6_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A12_8_6_LIBTOOLING_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace autosar {
namespace rule_A12_8_6 {
namespace libtooling {

class CollectBasesCallback;
class CheckBasesCallback;

class Checker {
 public:
  void InitCollectBasesCallback(analyzer::proto::ResultsList* results_list);
  void InitCheckBasesCallback();

  clang::ast_matchers::MatchFinder* GetMatchFinder1() { return &finder1_; }
  clang::ast_matchers::MatchFinder* GetMatchFinder2() { return &finder2_; }

 private:
  CollectBasesCallback* callback1_ = nullptr;
  CheckBasesCallback* callback2_ = nullptr;
  clang::ast_matchers::MatchFinder finder1_{};
  clang::ast_matchers::MatchFinder finder2_{};
  analyzer::proto::ResultsList* results_list_ = nullptr;
};
}  // namespace libtooling
}  // namespace rule_A12_8_6
}  // namespace autosar
#endif  // ANALYZER_AUTOSAR_rule_A12_8_6_LIBTOOLING_CHECKER_H_
