/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1179_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1179_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1179 {
namespace libtooling {
class BinopArithCallback;
class ArraySubcriptCallback;
class CompareCallback;
class BitwiseCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }
  void Run();

 private:
  BinopArithCallback* binop_callback_;
  ArraySubcriptCallback* arr_callback_;
  CompareCallback* cmp_callback_;
  BitwiseCallback* bitwise_callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace g1179
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1179_LIBTOOLING_CHECKER_H_
