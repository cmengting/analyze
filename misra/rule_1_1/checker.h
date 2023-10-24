/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_rule_1_1_CHECKER_H_
#define ANALYZER_MISRA_rule_1_1_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>

#include "misra/proto_util.h"

namespace misra {
namespace rule_1_1 {

struct LimitList {
  int struct_member_limit;
  int function_parm_limit;
  int function_arg_limit;
  int nested_record_limit;
  int nested_expr_limit;
  int switch_case_limit;
  int enum_constant_limit;
  int string_char_limit;
  int extern_id_limit;
};

class StructMemberCallback;
class FunctionParmCallback;
class FunctionArgCallback;
class NestedRecordCallback;
class NestedExprCallback;
class SwitchCaseCallback;
class EnumConstantCallback;
class StringCharCallback;
class ExternIDCallback;

/**
 *
 * Only report when the number of members of a struct is greater than the given
 * limitation.
 *
 */

class Checker {
 public:
  void Init(LimitList limits, analyzer::proto::ResultsList* results_list);

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  StructMemberCallback* struct_member_callback_;
  FunctionParmCallback* function_parm_callback_;
  FunctionArgCallback* function_arg_callback_;
  NestedRecordCallback* nested_record_callback_;
  NestedExprCallback* nested_expr_callback_;
  SwitchCaseCallback* switch_case_callback_;
  EnumConstantCallback* enum_constant_callback_;
  StringCharCallback* string_char_callback_;
  ExternIDCallback* extern_id_callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_1_1
}  // namespace misra

#endif  // ANALYZER_MISRA_rule_1_1_CHECKER_H_
