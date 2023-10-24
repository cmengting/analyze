/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_GOOGLECPP_G1157_LIBTOOLING_CHECKER_H_
#define ANALYZER_GOOGLECPP_G1157_LIBTOOLING_CHECKER_H_

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include "misra/proto_util.h"

using namespace clang;

namespace googlecpp {
namespace g1157 {
namespace libtooling {
class Check : public PPCallbacks {
 public:
  void Init(analyzer::proto::ResultsList* results_list_,
            SourceManager* source_manager);

  void InclusionDirective(SourceLocation HashLoc, const Token& IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          Optional<FileEntryRef> File, StringRef SearchPath,
                          StringRef RelativePath, const Module* Imported,
                          SrcMgr::CharacteristicKind FileType) override;

 private:
  SourceManager* source_manager_;
  analyzer::proto::ResultsList* results_list_;
};

class Action : public ASTFrontendAction {
 public:
  Action(analyzer::proto::ResultsList* results_list)
      : results_list_(results_list) {}
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
};

class Checker : public tooling::FrontendActionFactory {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<Action>(results_list_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace libtooling
}  // namespace g1157
}  // namespace googlecpp

#endif  // ANALYZER_GOOGLECPP_G1157_LIBTOOLING_CHECKER_H_
