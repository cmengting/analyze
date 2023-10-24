/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_rule_14_7_1_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_rule_14_7_1_LIBTOOLING_CHECKER_H_

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace misra_cpp_2008 {
namespace rule_14_7_1 {
namespace libtooling {
class ClassTemplateDeclCallback;
class ClassTemplateInstanceCallback;
class FuncTemplateInitCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  ClassTemplateInstanceCallback* class_template_callback_;
  ClassTemplateDeclCallback* class_template_decl_callback_;
  FuncTemplateInitCallback* func_template_callback_;
  ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_14_7_1
}  // namespace misra_cpp_2008

#endif  // ANALYZER_MISRA_CPP_2008_rule_14_7_1_LIBTOOLING_CHECKER_H_
