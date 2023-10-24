/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1201_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1201_LIBTOOLING_CHECKER_H_

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1201 {
namespace libtooling {
class Check : public PPCallbacks {
 public:
  void Init(analyzer::proto::ResultsList* results_list_,
            SourceManager* source_manager, std::string macro_prefix);

  void MacroDefined(const Token& macro_name_tok,
                    const MacroDirective* md) override;

 private:
  SourceManager* source_manager_;
  analyzer::proto::ResultsList* results_list_;
  std::string macro_prefix_;
};

class Action : public ASTFrontendAction {
 public:
  Action(analyzer::proto::ResultsList* results_list, std::string macro_preix)
      : results_list_(results_list), macro_preix_(macro_preix) {}
  std::unique_ptr<ASTConsumer> newASTConsumer() {
    return std::make_unique<ASTConsumer>();
  }
  std::unique_ptr<ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci,
                                                 llvm::StringRef in_file) {
    return std::make_unique<ASTConsumer>();
  }

  bool BeginSourceFileAction(CompilerInstance& ci);

 private:
  analyzer::proto::ResultsList* results_list_;
  std::string macro_preix_;
};

class Checker : public tooling::FrontendActionFactory {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            llvm::StringRef project_name);
  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<Action>(results_list_, macro_preix_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
  std::string macro_preix_;
};

}  // namespace libtooling
}  // namespace g1201
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1201_LIBTOOLING_CHECKER_H_
