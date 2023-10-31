/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_rule_1_1_CHECKER_H_
#define ANALYZER_MISRA_rule_1_1_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>

#include "misra/proto_util.h"

using analyzer::proto::ResultsList;
using namespace std;
using namespace clang;

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
  int macro_id_limit;
  int macro_parm_limit;
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

class ASTChecker {
 public:
  void Init(LimitList limits, ResultsList* results_list);

  ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

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
  ast_matchers::MatchFinder finder_;
  ResultsList* results_list_;
};

class PreprocessConsumer : public ASTConsumer {
 public:
  explicit PreprocessConsumer(ASTContext* context, ResultsList* results_list,
                              LimitList limits, CompilerInstance& compiler)
      : results_list_(results_list), limits_(limits), compiler_(compiler) {}

  virtual void HandleTranslationUnit(ASTContext& context);

 private:
  ResultsList* results_list_;
  LimitList limits_;
  CompilerInstance& compiler_;
};

class PreprocessAction : public ASTFrontendAction {
 public:
  PreprocessAction(ResultsList* results_list, LimitList limits)
      : results_list_(results_list), limits_(limits) {}
  virtual unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& compiler,
                                                    StringRef infile) {
    return make_unique<PreprocessConsumer>(&compiler.getASTContext(),
                                           results_list_, limits_, compiler);
  }

 private:
  ResultsList* results_list_;
  LimitList limits_;
};

class PreprocessChecker : public tooling::FrontendActionFactory {
 public:
  PreprocessChecker(ResultsList* results_list, LimitList limits)
      : results_list_(results_list), limits_(limits) {}
  unique_ptr<FrontendAction> create() override {
    return std::make_unique<PreprocessAction>(results_list_, limits_);
  }

 private:
  ResultsList* results_list_;
  LimitList limits_;
};

}  // namespace rule_1_1
}  // namespace misra

#endif  // ANALYZER_MISRA_rule_1_1_CHECKER_H_
