/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_6_5_5/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::libtooling_utils;
using namespace llvm;

namespace misra_cpp_2008 {
namespace rule_6_5_5 {
namespace libtooling {
class ForStmtCallback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    auto loop_counter_matcher =
        CreateLoopCounterMatcher(AllCondFormat, AllIncFormat);
    auto loop_var_matcher = findAll(
        declRefExpr(unless(to(varDecl(equalsBoundNode("loop_counter")))),
                    to(varDecl().bind("loop_var")))
            .bind("var_ref"));
    auto assign_matcher = CreateAssignmentMatcher(
        ForIncrementVarFormat(AllIncFormat), "loop_var", "assign_var");

    finder->addMatcher(forStmt(loop_counter_matcher,
                               anyOf(hasCondition(loop_var_matcher),
                                     hasIncrement(loop_var_matcher)),
                               anyOf(hasCondition(assign_matcher),
                                     hasIncrement(assign_matcher)))
                           .bind("for_stmt"),
                       this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Stmt* for_stmt = result.Nodes.getNodeAs<Stmt>("for_stmt");
    const DeclRefExpr* var_ref = result.Nodes.getNodeAs<DeclRefExpr>("var_ref");

    if (misra::libtooling_utils::IsInSystemHeader(for_stmt, result.Context)) {
      return;
    }

    if (var_ref->getType().isVolatileQualified()) {
      return;
    }

    string error_message =
        "不得在条件或表达式中修改循环控制变量（循环计数器除外）";
    string path =
        misra::libtooling_utils::GetFilename(var_ref, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(var_ref, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_6_5_5);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

class WhileStmtCallback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    auto loop_var_matcher =
        findAll(declRefExpr(to(varDecl().bind("loop_var"))).bind("var_ref"));
    auto assign_matcher = CreateAssignmentMatcher(
        ForIncrementVarFormat(AllIncFormat), "loop_var", "assign_var");

    finder->addMatcher(
        whileStmt(hasCondition(loop_var_matcher), hasCondition(assign_matcher))
            .bind("while_stmt"),
        this);

    finder->addMatcher(
        doStmt(hasCondition(loop_var_matcher), hasCondition(assign_matcher))
            .bind("while_stmt"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Stmt* while_stmt = result.Nodes.getNodeAs<Stmt>("while_stmt");
    const DeclRefExpr* var_ref = result.Nodes.getNodeAs<DeclRefExpr>("var_ref");

    if (misra::libtooling_utils::IsInSystemHeader(while_stmt, result.Context)) {
      return;
    }

    if (var_ref->getType().isVolatileQualified() ||
        stmt_set.count(while_stmt) > 0) {
      return;
    }

    stmt_set.insert(while_stmt);
    string error_message =
        "不得在条件或表达式中修改循环控制变量（循环计数器除外）";
    string path =
        misra::libtooling_utils::GetFilename(var_ref, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(var_ref, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_6_5_5);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
  unordered_set<const Stmt*> stmt_set;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  forStmtCallback_ = new ForStmtCallback;
  whileStmtCallback_ = new WhileStmtCallback;
  forStmtCallback_->Init(result_list, &finder_);
  whileStmtCallback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_6_5_5
}  // namespace misra_cpp_2008
