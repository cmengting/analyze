/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A26_5_2/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/QualTypeNames.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  string error_message =
      "Random number engines shall not be default-initialized.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A26_5_2 {
namespace libtooling {
class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(varDecl(unless(isExpansionInSystemHeader()),
                               has(cxxConstructExpr().bind("ce")))
                           .bind("vd"),
                       this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const VarDecl* vd = result.Nodes.getNodeAs<VarDecl>("vd");
    const CXXConstructExpr* ce = result.Nodes.getNodeAs<CXXConstructExpr>("ce");

    if (vd && ce) {
      string type =
          vd->getType().getDesugaredType(*result.Context).getAsString();
      bool isRandomEngine =
          (type.find("std::linear_congruential_engine") != string::npos) ||
          (type.find("std::mersenne_twister_engine") != string::npos) ||
          (type.find("std::subtract_with_carry_engine") != string::npos) ||
          (type.find("std::discard_block_engine") != string::npos) ||
          (type.find("std::independent_bits_engine") != string::npos) ||
          (type.find("std::shuffle_order_engine") != string::npos);
      if (isRandomEngine && ce->getNumArgs() == 0) {
        ReportError(
            misra::libtooling_utils::GetFilename(vd, result.SourceManager),
            misra::libtooling_utils::GetLine(vd, result.SourceManager),
            results_list_);
      }
    }
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
}  // namespace rule_A26_5_2
}  // namespace autosar
