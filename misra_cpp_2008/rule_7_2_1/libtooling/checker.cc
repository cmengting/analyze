/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_7_2_1/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace misra_cpp_2008 {
namespace rule_7_2_1 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  // check the case to find if we cast to an enum type
  // rule: 底层类型为enum的表达式的值必须与该枚举的枚举器相对应
  // if the values not corresponding to the enumerators of the enumeration,
  // there must exists a type cast in CXX.
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        castExpr(unless(hasCastKind(CK_LValueToRValue))).bind("castToEnum"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) override {
    const Expr* cast = result.Nodes.getNodeAs<Expr>("castToEnum");
    if (!cast) {
      return;
    }
    if (misra::libtooling_utils::IsInSystemHeader(cast, result.Context)) {
      return;
    }
    QualType destination_type = cast->getType();
    // check if destination type is enum
    if (!destination_type->isEnumeralType()) {
      return;
    }
    string error_message =
        "底层类型为enum的表达式的值必须与该枚举的枚举器相对应";
    string path =
        misra::libtooling_utils::GetFilename(cast, result.SourceManager);
    int line = misra::libtooling_utils::GetLine(cast, result.SourceManager);
    analyzer::proto::Result* pb_result =
        misra::proto_util::AddResultToResultsList(results_list_, path, line,
                                                  error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_7_2_1);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  callback_ = new Callback;
  callback_->Init(result_list, &finder_);
}
}  // namespace libtooling
}  // namespace rule_7_2_1
}  // namespace misra_cpp_2008
