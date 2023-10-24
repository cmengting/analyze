/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_RULE_3_2_1_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_RULE_3_2_1_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

/**
 * this checker assume that two types are compatible iff
 * two types are identical.
 * the only expecition is array:
 * int arr[] will be compatible with any int array.
 */

namespace {
struct DeclInfo;
struct VarDeclInfo;
struct FuncDeclInfo;
}  // namespace
namespace misra_cpp_2008 {
namespace rule_3_2_1 {
namespace libtooling {
class VarCallback;
class FuncCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  VarCallback* var_callback_;
  FuncCallback* func_callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_3_2_1
}  // namespace misra_cpp_2008

#endif  // ANALYZER_MISRA_CPP_2008_RULE_3_2_1_LIBTOOLING_CHECKER_H_
