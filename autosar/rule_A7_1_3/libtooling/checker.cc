/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A7_1_3/libtooling/checker.h"

#include <clang/Lex/Lexer.h>
#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list, string str = "") {
  std::string error_message =
      "CV-qualifiers shall be placed on the right hand side of the type that is a typedef or a using name." +
      str;
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A7_1_3 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(
        valueDecl(
            unless(isExpansionInSystemHeader()),
            hasType(qualType(anyOf(isConstQualified(), isVolatileQualified()))),
            hasType(qualType(
                hasDeclaration(anyOf(typedefDecl(), typeAliasDecl())))))
            .bind("vd"),
        this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const ValueDecl* vd = result.Nodes.getNodeAs<ValueDecl>("vd");
    if (vd) {
      const QualType qt = vd->getType().getLocalUnqualifiedType();
      string fullName =
          Lexer::getSourceText(
              CharSourceRange::getTokenRange(vd->getSourceRange()),
              result.Context->getSourceManager(), result.Context->getLangOpts())
              .str();
      string qtName = qt.getAsString();
      if ((fullName.find("const") < fullName.find(qtName)) ||
          (fullName.find("volatile") < fullName.find(qtName))) {
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
}  // namespace rule_A7_1_3
}  // namespace autosar
