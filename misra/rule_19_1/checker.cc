/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_19_1/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace misra {
namespace rule_19_1 {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        binaryOperator(
            hasLHS(hasDescendant(declRefExpr(
                to(varDecl().bind("LDecl")),
                hasParent(memberExpr(member(decl().bind("LMem"))))))),
            hasRHS(hasDescendant(declRefExpr(
                to(varDecl(equalsBoundNode("LDecl"))),
                unless(
                    hasParent(memberExpr(member(equalsBoundNode("LMem")))))))))
            .bind("op"),
        this);
    finder->addMatcher(
        cxxOperatorCallExpr(
            hasLHS(hasDescendant(declRefExpr(
                to(varDecl().bind("LDecl")),
                hasParent(memberExpr(member(decl().bind("LMem"))))))),
            hasRHS(hasDescendant(declRefExpr(
                to(varDecl(equalsBoundNode("LDecl"))),
                unless(
                    hasParent(memberExpr(member(equalsBoundNode("LMem")))))))))
            .bind("op"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Expr* e = result.Nodes.getNodeAs<Expr>("op");
    const VarDecl* decl = result.Nodes.getNodeAs<VarDecl>("LDecl");
    if (misra::libtooling_utils::IsInSystemHeader(e, result.Context)) {
      return;
    }

    if (!decl->getType()->isUnionType()) return;

    string error_message =
        "[C0302][misra-c2012-19.1]: An object shall not be assigned or copied to an overlapping object";
    string path = misra::libtooling_utils::GetFilename(e, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(e, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_19_1);
    LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                                 line);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace rule_19_1
}  // namespace misra
