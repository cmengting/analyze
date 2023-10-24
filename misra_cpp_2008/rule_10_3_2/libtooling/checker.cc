/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra_cpp_2008/rule_10_3_2/libtooling/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {
void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message = "每个覆盖的虚函数都应使用 virtual 关键字声明";
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_CPP_2008_RULE_10_3_2);
}
}  // namespace

namespace misra_cpp_2008 {
namespace rule_10_3_2 {
namespace libtooling {
class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(cxxMethodDecl(isVirtual(), unless(isVirtualAsWritten()))
                           .bind("virtual_method"),
                       this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const CXXMethodDecl* method_decl =
        result.Nodes.getNodeAs<CXXMethodDecl>("virtual_method");
    if (method_decl->isDefaulted()) {
      return;
    }
    string path =
        misra::libtooling_utils::GetFilename(method_decl, result.SourceManager);
    int line_number =
        misra::libtooling_utils::GetLine(method_decl, result.SourceManager);
    ReportError(path, line_number, results_list_);
  }

 private:
  ResultsList* results_list_;
};

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace libtooling
}  // namespace rule_10_3_2
}  // namespace misra_cpp_2008
