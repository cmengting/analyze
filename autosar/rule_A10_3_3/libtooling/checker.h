/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A10_3_3_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A10_3_3_LIBTOOLING_CHECKER_H_

#include <tuple>
#include <unordered_map>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;
using MatchFinder = clang::ast_matchers::MatchFinder;

using analyzer::proto::ResultsList;

namespace autosar {
namespace rule_A10_3_3 {
namespace libtooling {
class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder);
  void run(const MatchFinder::MatchResult& result);

 private:
  ResultsList* results_list_;
  std::unordered_map<std::string, std::vector<std::tuple<std::string, int>>>
      unnamed_lambda_map_;
};

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
}  // namespace rule_A10_3_3
}  // namespace autosar

#endif
