/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_3_1_3/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace misra_cpp_2008 {
namespace rule_3_1_3 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(varDecl(hasType(arrayType())).bind("vd"), this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const VarDecl* vd = result.Nodes.getNodeAs<VarDecl>("vd");
    if (misra::libtooling_utils::IsInSystemHeader(vd, result.Context)) {
      return;
    }
    if (!vd->getType()->isIncompleteArrayType()) {
      return;
    }
    string error_message =
        "声明一个数组时，应该显式说明其大小，或通过初始化隐式定义其大小";
    string path =
        misra::libtooling_utils::GetFilename(vd, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(vd, result.SourceManager);
    misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                              error_message);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_3_1_3
}  // namespace misra_cpp_2008
