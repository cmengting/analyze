/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1181_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1181_LIBTOOLING_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include "misra/proto_util.h"

namespace googlecpp {
namespace g1181 {
namespace libtooling {

class Callback : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            clang::ast_matchers::MatchFinder* finder);

  void run(
      const clang::ast_matchers::MatchFinder::MatchResult& result) override;

 private:
  analyzer::proto::ResultsList* results_list_;
};

class ASTChecker {
 public:
  void Init(analyzer::proto::ResultsList* results_list) {
    results_list_ = results_list;
    callback_ = new Callback;
    callback_->Init(results_list_, &finder_);
  }
  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  Callback* callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace g1181
}  // namespace googlecpp

#endif
