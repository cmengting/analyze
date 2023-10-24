/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_9_6_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace misra_cpp_2008 {
namespace rule_9_6_4 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;

    finder->addMatcher(
        fieldDecl(allOf(hasType(isInteger()), hasBitWidth(1))).bind("decl"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const ValueDecl* s = result.Nodes.getNodeAs<ValueDecl>("decl");

    // check if it is signed integer type
    auto type = s->getType();
    if (!type->isSignedIntegerType()) {
      return;
    }

    // check if the field declaration has a name
    const ValueDecl* v = result.Nodes.getNodeAs<ValueDecl>("decl");
    if ((v->getNameAsString()).empty()) {
      return;
    }

    if (misra::libtooling_utils::IsInSystemHeader(s, result.Context)) {
      return;
    }

    string error_message = "带符号整数类型的signed命名位域的长度应大于一位";
    string path = misra::libtooling_utils::GetFilename(s, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(s, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_9_6_4);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_9_6_4
}  // namespace misra_cpp_2008
