/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A18_1_4/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace misra::libtooling_utils;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message =
      "A pointer pointing to an element of an array of objects shall not be passed to a smart pointer of single object type.";
  AddResultToResultsList(results_list, path, line_number, error_message);
}

}  // namespace

namespace autosar {
namespace rule_A18_1_4 {
namespace libtooling {

class Callback : public MatchFinder::MatchCallback {
 public:
  void Init(ResultsList* results_list, MatchFinder* finder) {
    results_list_ = results_list;
    Matcher<Stmt> share_ptr_ref =
        declRefExpr(hasDeclaration(varDecl(hasType(qualType(hasDeclaration(
            classTemplateSpecializationDecl(matchesName("::std::shared_ptr"),
                                            isExpansionInSystemHeader())))))));
    Matcher<Stmt> unique_ptr_ref =
        declRefExpr(hasDeclaration(varDecl(hasType(qualType(hasDeclaration(
            classTemplateSpecializationDecl(matchesName("::std::unique_ptr"),
                                            isExpansionInSystemHeader())))))));
    Matcher<Stmt> array_ref =
        declRefExpr(
            to(varDecl(hasDescendant(declRefExpr(hasDeclaration(functionDecl(
                anyOf(hasName("make_unique"), hasName("make_shared")),
                hasAnyTemplateArgument(
                    templateArgument(refersToType(arrayType()))))))))))
            .bind("decl_ref");
    Matcher<Stmt> new_array_ref =
        declRefExpr(to(varDecl(has(cxxNewExpr(isArray()))))).bind("decl_ref");
    Matcher<Stmt> container_ref =
        declRefExpr(hasType(cxxRecordDecl(hasAnyName(
                        "::std::vector", "::std::set", "::std::unordered_set",
                        "::std::map", "::std::unordered_map", "::std::array",
                        "::std::deque"))))
            .bind("decl_ref");
    finder->addMatcher(
        declStmt(has(varDecl(hasDescendant(cxxConstructExpr(anyOf(
                     hasDescendant(array_ref), hasDescendant(new_array_ref),
                     hasDescendant(container_ref)))))),
                 unless(isExpansionInSystemHeader())),
        this);
    finder->addMatcher(
        cxxMemberCallExpr(
            has(memberExpr(anyOf(has(unique_ptr_ref), has(share_ptr_ref)))),
            hasDescendant(array_ref), unless(isExpansionInSystemHeader())),
        this);
  }

  void run(const MatchFinder::MatchResult& result) {
    const DeclRefExpr* decl_ref =
        result.Nodes.getNodeAs<DeclRefExpr>("decl_ref");
    ReportError(GetFilename(decl_ref, result.SourceManager),
                GetLine(decl_ref, result.SourceManager), results_list_);
  }

 private:
  ResultsList* results_list_;
};

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

}  // namespace libtooling
}  // namespace rule_A18_1_4
}  // namespace autosar
