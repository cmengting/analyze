/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/dir_4_11/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace misra {
namespace dir_4_11 {

class CallCallback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    auto calleeMatcher = callee(functionDecl(hasName("calloc")));
    auto argumentMatcher = hasArgument(0, expr().bind("x"));
    finder->addMatcher(callExpr(calleeMatcher, argumentMatcher), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    ASTContext* context = result.Context;
    const Expr* first_arg = result.Nodes.getNodeAs<Expr>("x");
    if (!first_arg) {
      return;
    }
    if (libtooling_utils::IsInSystemHeader(first_arg, result.Context)) {
      return;
    }

    if (first_arg->getType()->isIntegerType()) {
      clang::Expr::EvalResult rint;
      first_arg->EvaluateAsInt(rint, *context);
      if (!rint.Val.isInt() || rint.Val.getInt() >= 0) {
        return;
      }
    }
    std::string error_message =
        "[C2314][misra-c2012-dir-4.11]: violation of misra-c2012-dir-4.11";
    analyzer::proto::Result* pb_result = AddResultToResultsList(
        results_list_,
        libtooling_utils::GetFilename(first_arg, result.SourceManager),
        libtooling_utils::GetLine(first_arg, result.SourceManager),
        error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_C_2012_DIR_4_11);
    pb_result->set_external_message(
        "The first argument of calloc should be not negative");
    LOG(INFO) << error_message;
  }

 private:
  ResultsList* results_list_;
  string GetFunctionName(const Decl* decl,
                         const SourceManager* source_manager) {
    return decl->getAsFunction()->getNameAsString();
  }
};

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new CallCallback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace dir_4_11
}  // namespace misra
