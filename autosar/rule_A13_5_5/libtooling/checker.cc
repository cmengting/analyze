/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A13_5_5/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;
using std::vector;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "Comparison operators shall be non-member functions with identical parameter types and noexcept.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A13_5_5 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        functionDecl(
            hasAnyOverloadedOperatorName("==", "!=", ">", "<", "<=", ">="),
            unless(isImplicit()), unless(isExpansionInSystemHeader()))
            .bind("fd"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    if (!fd) return;
    const CXXMethodDecl* cmd = dynamic_cast<const CXXMethodDecl*>(fd);
    const Type* parm1_type = nullptr;
    const Type* parm2_type = nullptr;
    if (fd->getNumParams() == 2) {
      parm1_type =
          fd->getParamDecl(0)->getType().getNonReferenceType().getTypePtr();
      parm2_type =
          fd->getParamDecl(1)->getType().getNonReferenceType().getTypePtr();
    }
    const ExceptionSpecificationType exception_type =
        fd->getExceptionSpecType();
    bool is_no_throw = exception_type == clang::EST_BasicNoexcept ||
                       exception_type == clang::EST_NoThrow ||
                       exception_type == clang::EST_NoexceptTrue;
    if (cmd || parm1_type != parm2_type || !is_no_throw)
      ReportError(
          misra::libtooling_utils::GetFilename(fd, result.SourceManager),
          misra::libtooling_utils::GetLine(fd, result.SourceManager),
          results_list_);
  }

 private:
  analyzer::proto::ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* result_list) {
  results_list_ = result_list;
  callback_ = new Callback;
  callback_->Init(results_list_, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A13_5_5
}  // namespace autosar
