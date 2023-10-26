/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_1_1/checker.h"

#include <glog/logging.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace std;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

const string error_message =
    "[C2201][misra-c2012-1.1]: The program shall contain no violations of the standard C syntax and constraints, and shall not exceed the implementation's translation limits";

void ReportStructMemberError(int struct_member_limit, int struct_member_count,
                             string struct_name, string path, int line_number,
                             ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_STRUCT_MEMBER);
  pb_result->set_struct_member_limit(to_string(struct_member_limit));
  pb_result->set_struct_member_count(to_string(struct_member_count));
  pb_result->set_name(struct_name);
}

void ReportFunctionParmError(int function_parm_limit, int function_parm_count,
                             string func_name, string path, int line_number,
                             ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_FUNCTION_PARM);
  pb_result->set_function_parm_limit(to_string(function_parm_limit));
  pb_result->set_function_parm_count(to_string(function_parm_count));
  pb_result->set_name(func_name);
}

void ReportFunctionArgError(int function_arg_limit, int function_arg_count,
                            string call_expr, string path, int line_number,
                            ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_FUNCTION_ARG);
  pb_result->set_function_arg_limit(to_string(function_arg_limit));
  pb_result->set_function_arg_count(to_string(function_arg_count));
  pb_result->set_name(call_expr);
}

void ReportNestedRecordError(int nested_record_limit, int nested_record_count,
                             string record_name, string path, int line_number,
                             ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_NESTED_RECORD);
  pb_result->set_nested_record_limit(to_string(nested_record_limit));
  pb_result->set_nested_record_count(to_string(nested_record_count));
  pb_result->set_name(record_name);
}

void ReportNestedExprError(int nested_expr_limit, int nested_expr_count,
                           string paren_expr, string path, int line_number,
                           ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_NESTED_EXPR);
  pb_result->set_nested_expr_limit(to_string(nested_expr_limit));
  pb_result->set_nested_expr_count(to_string(nested_expr_count));
  pb_result->set_name(paren_expr);
}

void ReportSwitchCaseError(int switch_case_limit, int switch_case_count,
                           string switch_stmt, string path, int line_number,
                           ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_SWITCH_CASE);
  pb_result->set_switch_case_limit(to_string(switch_case_limit));
  pb_result->set_switch_case_count(to_string(switch_case_count));
  pb_result->set_name(switch_stmt);
}

void ReportEnumConstantError(int enum_constant_limit, int enum_constant_count,
                             string enum_name, string path, int line_number,
                             ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_ENUM_CONSTANT);
  pb_result->set_enum_constant_limit(to_string(enum_constant_limit));
  pb_result->set_enum_constant_count(to_string(enum_constant_count));
  pb_result->set_name(enum_name);
}

void ReportStringCharError(int string_char_limit, int string_char_count,
                           string this_str, string path, int line_number,
                           ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_STRING_CHAR);
  pb_result->set_string_char_limit(to_string(string_char_limit));
  pb_result->set_string_char_count(to_string(string_char_count));
  pb_result->set_name(this_str);
}

void ReportExternIDError(int extern_id_limit, int extern_id_count, string path,
                         int line_number, ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_EXTERN_ID);
  pb_result->set_extern_id_limit(to_string(extern_id_limit));
  pb_result->set_extern_id_count(to_string(extern_id_count));
}

void ReportMacroIDError(int macro_id_limit, int macro_id_count, string path,
                        int line_number, ResultsList* results_list) {
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_1_1_MACRO_ID);
  pb_result->set_macro_id_limit(to_string(macro_id_limit));
  pb_result->set_macro_id_count(to_string(macro_id_count));
}

}  // namespace

namespace misra {
namespace rule_1_1 {

class StructMemberCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int struct_member_limit, ResultsList* results_list,
            MatchFinder* finder) {
    struct_member_limit_ = struct_member_limit;
    results_list_ = results_list;
    finder->addMatcher(
        recordDecl(unless(isExpansionInSystemHeader())).bind("rd"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const RecordDecl* rd = result.Nodes.getNodeAs<RecordDecl>("rd");
    int struct_member_count = 0;
    for (auto it = rd->field_begin(); it != rd->field_end(); ++it) {
      struct_member_count += 1;
    }
    if (struct_member_count > struct_member_limit_) {
      ReportStructMemberError(
          struct_member_limit_, struct_member_count,
          rd->getQualifiedNameAsString(),
          libtooling_utils::GetFilename(rd, result.SourceManager),
          libtooling_utils::GetLine(rd, result.SourceManager), results_list_);
    }
  }

 private:
  int struct_member_limit_;
  ResultsList* results_list_;
};

class FunctionParmCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int function_parm_limit, ResultsList* results_list,
            MatchFinder* finder) {
    function_parm_limit_ = function_parm_limit;
    results_list_ = results_list;
    finder->addMatcher(
        functionDecl(unless(isExpansionInSystemHeader())).bind("fd"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    int function_parm_count = fd->getNumParams();
    if (function_parm_count > function_parm_limit_) {
      ReportFunctionParmError(
          function_parm_limit_, function_parm_count,
          fd->getQualifiedNameAsString(),
          libtooling_utils::GetFilename(fd, result.SourceManager),
          libtooling_utils::GetLine(fd, result.SourceManager), results_list_);
    }
  }

 private:
  int function_parm_limit_;
  ResultsList* results_list_;
};

class FunctionArgCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int function_arg_limit, ResultsList* results_list,
            MatchFinder* finder) {
    function_arg_limit_ = function_arg_limit;
    results_list_ = results_list;
    finder->addMatcher(callExpr(unless(isExpansionInSystemHeader())).bind("ce"),
                       this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const CallExpr* ce = result.Nodes.getNodeAs<CallExpr>("ce");
    int function_arg_count = ce->getNumArgs();
    if (function_arg_count > function_arg_limit_) {
      ReportFunctionArgError(
          function_arg_limit_, function_arg_count,
          misra::libtooling_utils::GetTokenFromSourceLoc(
              result.SourceManager, ce->getBeginLoc(), ce->getEndLoc()),
          libtooling_utils::GetFilename(ce, result.SourceManager),
          libtooling_utils::GetLine(ce, result.SourceManager), results_list_);
    }
  }

 private:
  int function_arg_limit_;
  ResultsList* results_list_;
};

class NestedRecordCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int nested_record_limit, ResultsList* results_list,
            MatchFinder* finder) {
    nested_record_limit_ = nested_record_limit;
    results_list_ = results_list;
    finder->addMatcher(
        recordDecl(unless(isExpansionInSystemHeader())).bind("rd"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const RecordDecl* rd = result.Nodes.getNodeAs<RecordDecl>("rd");
    int depth = 0;
    const RecordDecl* current = rd;
    while (current) {
      depth++;
      if (depth > nested_record_limit_) {
        ReportNestedRecordError(
            nested_record_limit_, depth, rd->getQualifiedNameAsString(),
            libtooling_utils::GetFilename(rd, result.SourceManager),
            libtooling_utils::GetLine(rd, result.SourceManager), results_list_);
        return;
      }
      current = dyn_cast_or_null<RecordDecl>(current->getLexicalParent());
    }
  }

 private:
  int nested_record_limit_;
  ResultsList* results_list_;
};

class NestedExprCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int nested_expr_limit, ResultsList* results_list,
            MatchFinder* finder) {
    nested_expr_limit_ = nested_expr_limit;
    results_list_ = results_list;
    finder->addMatcher(
        parenExpr(unless(isExpansionInSystemHeader())).bind("pe"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const ParenExpr* pe = result.Nodes.getNodeAs<ParenExpr>("pe");
    int depth = 0;
    const ParenExpr* current = pe;
    while (current) {
      depth++;
      if (depth > nested_expr_limit_) {
        ReportNestedExprError(
            nested_expr_limit_, depth,
            misra::libtooling_utils::GetTokenFromSourceLoc(
                result.SourceManager, pe->getBeginLoc(), pe->getEndLoc()),
            libtooling_utils::GetFilename(pe, result.SourceManager),
            libtooling_utils::GetLine(pe, result.SourceManager), results_list_);
        return;
      }
      current = dyn_cast_or_null<ParenExpr>(current->getSubExpr());
    }
  }

 private:
  int nested_expr_limit_;
  ResultsList* results_list_;
};

class SwitchCaseCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int switch_case_limit, ResultsList* results_list,
            MatchFinder* finder) {
    switch_case_limit_ = switch_case_limit;
    results_list_ = results_list;
    finder->addMatcher(
        switchStmt(unless(isExpansionInSystemHeader())).bind("ss"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const SwitchStmt* ss = result.Nodes.getNodeAs<SwitchStmt>("ss");
    int case_count = 0;
    const SwitchCase* sc = ss->getSwitchCaseList();
    while (sc) {
      case_count++;
      if (case_count > switch_case_limit_) {
        ReportSwitchCaseError(
            switch_case_limit_, case_count,
            misra::libtooling_utils::GetTokenFromSourceLoc(
                result.SourceManager, ss->getBeginLoc(), ss->getEndLoc()),
            libtooling_utils::GetFilename(ss, result.SourceManager),
            libtooling_utils::GetLine(ss, result.SourceManager), results_list_);
        return;
      }
      sc = sc->getNextSwitchCase();
    }
  }

 private:
  int switch_case_limit_;
  ResultsList* results_list_;
};

class EnumConstantCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int enum_constant_limit, ResultsList* results_list,
            MatchFinder* finder) {
    enum_constant_limit_ = enum_constant_limit;
    results_list_ = results_list;
    finder->addMatcher(enumDecl(unless(isExpansionInSystemHeader())).bind("ed"),
                       this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const EnumDecl* ed = result.Nodes.getNodeAs<EnumDecl>("ed");
    int enum_constant_count = 0;
    for (auto enumerator : ed->enumerators()) {
      enum_constant_count++;
    }
    if (enum_constant_count > enum_constant_limit_) {
      ReportEnumConstantError(
          enum_constant_limit_, enum_constant_count,
          ed->getQualifiedNameAsString(),
          libtooling_utils::GetFilename(ed, result.SourceManager),
          libtooling_utils::GetLine(ed, result.SourceManager), results_list_);
    }
  }

 private:
  int enum_constant_limit_;
  ResultsList* results_list_;
};

class StringCharCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int string_char_limit, ResultsList* results_list,
            MatchFinder* finder) {
    string_char_limit_ = string_char_limit;
    results_list_ = results_list;
    finder->addMatcher(
        stringLiteral(unless(isExpansionInSystemHeader())).bind("sl"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const StringLiteral* sl = result.Nodes.getNodeAs<StringLiteral>("sl");
    int string_char_count = sl->getLength();
    if (string_char_count > string_char_limit_) {
      ReportStringCharError(
          string_char_limit_, string_char_count, sl->getString().str(),
          libtooling_utils::GetFilename(sl, result.SourceManager),
          libtooling_utils::GetLine(sl, result.SourceManager), results_list_);
    }
  }

 private:
  int string_char_limit_;
  ResultsList* results_list_;
};

class ExternIDCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int extern_id_limit, ResultsList* results_list,
            MatchFinder* finder) {
    extern_id_limit_ = extern_id_limit;
    results_list_ = results_list;
    finder->addMatcher(
        translationUnitDecl(unless(isExpansionInSystemHeader())).bind("tud"),
        this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const TranslationUnitDecl* tud =
        result.Nodes.getNodeAs<TranslationUnitDecl>("tud");
    misra::libtooling_utils::ASTVisitor visitor;
    visitor.TraverseDecl(const_cast<TranslationUnitDecl*>(tud));
    int extern_id_count = 0;
    for (const VarDecl* vd : visitor.getVarDecls()) {
      if (vd->hasExternalFormalLinkage()) extern_id_count++;
    }
    for (const FunctionDecl* fd : visitor.getFuncDecls()) {
      if (fd->hasExternalFormalLinkage()) extern_id_count++;
    }
    if (extern_id_count > extern_id_limit_) {
      ReportExternIDError(
          extern_id_count, extern_id_limit_,
          libtooling_utils::GetFilename(tud, result.SourceManager),
          libtooling_utils::GetLine(tud, result.SourceManager), results_list_);
    }
  }

 private:
  int extern_id_limit_;
  ResultsList* results_list_;
};

void ASTChecker::Init(LimitList limits, ResultsList* results_list) {
  results_list_ = results_list;
  struct_member_callback_ = new StructMemberCallback;
  struct_member_callback_->Init(limits.struct_member_limit, results_list_,
                                &finder_);
  function_parm_callback_ = new FunctionParmCallback;
  function_parm_callback_->Init(limits.function_parm_limit, results_list_,
                                &finder_);
  function_arg_callback_ = new FunctionArgCallback;
  function_arg_callback_->Init(limits.function_arg_limit, results_list_,
                               &finder_);
  nested_record_callback_ = new NestedRecordCallback;
  nested_record_callback_->Init(limits.nested_record_limit, results_list_,
                                &finder_);
  nested_expr_callback_ = new NestedExprCallback;
  nested_expr_callback_->Init(limits.nested_expr_limit, results_list_,
                              &finder_);
  switch_case_callback_ = new SwitchCaseCallback;
  switch_case_callback_->Init(limits.switch_case_limit, results_list_,
                              &finder_);
  enum_constant_callback_ = new EnumConstantCallback;
  enum_constant_callback_->Init(limits.enum_constant_limit, results_list_,
                                &finder_);
  string_char_callback_ = new StringCharCallback;
  string_char_callback_->Init(limits.string_char_limit, results_list_,
                              &finder_);
  extern_id_callback_ = new ExternIDCallback;
  extern_id_callback_->Init(limits.extern_id_limit, results_list_, &finder_);
}

void PreprocessConsumer::HandleTranslationUnit(ASTContext& context) {
  Preprocessor& pp = compiler_.getPreprocessor();
  SourceManager& sm = context.getSourceManager();
  const TranslationUnitDecl* tud = context.getTranslationUnitDecl();
  set<string> macro_ids{};
  for (const auto& macro : pp.macros()) {
    const MacroInfo* info = pp.getMacroInfo(macro.getFirst());
    if (!info) continue;
    SourceLocation sl = info->getDefinitionLoc();
    if (sl.isValid() && !sm.isInSystemHeader(sl) && !sm.isInSystemMacro(sl) &&
        !info->isBuiltinMacro() && sm.isInMainFile(sl)) {
      string macro_id = macro.first->getName().str();
      macro_ids.insert(macro_id);
    }
  }
  if (macro_ids.size() > limits_.macro_id_limit)
    ReportMacroIDError(limits_.macro_id_limit, macro_ids.size(),
                       libtooling_utils::GetFilename(tud, &sm),
                       libtooling_utils::GetLine(tud, &sm), results_list_);
}

}  // namespace rule_1_1
}  // namespace misra
