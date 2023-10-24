/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A7_3_1_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A7_3_1_LIBTOOLING_CHECKER_H_

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace autosar {
namespace rule_A7_3_1 {
namespace libtooling {

using analyzer::proto::ResultsList;

class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder);
  void run(const MatchFinder::MatchResult& result);

 private:
  ResultsList* results_list_;
  std::unordered_map<std::string, std::vector<std::tuple<std::string, int>>>
      statics_map_;
  std::unordered_map<std::string, unsigned int> named_map_;
};

class Checker {
 public:
  void Init(ResultsList* results_list);
  MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  Callback* callback_;
  MatchFinder finder_;
  ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace rule_A7_3_1
}  // namespace autosar

#endif
