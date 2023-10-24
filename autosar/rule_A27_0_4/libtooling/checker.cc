/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A27_0_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message = "C-style string shall not be used.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A27_0_4 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        varDecl(hasType(arrayType(hasElementType(isAnyCharacter()))),
                hasInitializer(initListExpr().bind("list")))
            .bind("vd"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const VarDecl* var_decl = result.Nodes.getNodeAs<VarDecl>("vd");
    const InitListExpr* list = result.Nodes.getNodeAs<InitListExpr>("list");
    if (misra::libtooling_utils::IsInSystemHeader(var_decl, result.Context)) {
      return;
    }
    if (var_decl && list) {
      if (list->getNumInits() < 1) {
        return;
      }
      const Expr* last_element = list->getInit(list->getNumInits() - 1);
      if (isa<StringLiteral>(last_element) ||
          (isa<CharacterLiteral>(last_element) &&
           cast<CharacterLiteral>(last_element)->getValue() == 0)) {
        std::string path = misra::libtooling_utils::GetFilename(
            var_decl, result.SourceManager);
        int line_number =
            misra::libtooling_utils::GetLine(var_decl, result.SourceManager);
        ReportError(path, line_number, results_list_);
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
}  // namespace rule_A27_0_4
}  // namespace autosar
