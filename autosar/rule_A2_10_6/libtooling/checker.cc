/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A2_10_6/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "clang/AST/StmtVisitor.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace ast_matchers;

namespace {

constexpr char kClassEnumString[] = "classOrEnumDecl";

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "A class or enumeration name shall not be hidden by a variable, function or enumerator declaration in the same scope.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}
}  // namespace

namespace autosar {
namespace rule_A2_10_6 {
namespace libtooling {

void Callback::Init(ResultsList* results_list, MatchFinder* finder) {
  results_list_ = results_list;
  finder->addMatcher(tagDecl().bind(kClassEnumString), this);
}

void Callback::run(const MatchFinder::MatchResult& result) {
  const TagDecl* tag_decl = result.Nodes.getNodeAs<TagDecl>(kClassEnumString);
  if (!(isa<EnumDecl>(tag_decl) ||
        (isa<CXXRecordDecl>(tag_decl) &&
         !cast<CXXRecordDecl>(tag_decl)->isImplicit()))) {
    // Only check EnumDecl and CXXRecordecl.
    // In the declaration of a class, it declares itself implicitly. So ignore
    // the implicit declaration to avoid unexpected errors.
    // So if it's an enum declaration or an explicit class declaration, skip it.
    return;
  }
  const DeclContext* ctx = tag_decl->getDeclContext();
  if (!ctx) {
    return;
  }
  for (auto decl : ctx->decls()) {
    // Check the sibling declarations.
    if (decl == tag_decl) {
      // Ignore itself.
      continue;
    } else if ((isa<VarDecl>(decl) || isa<FunctionDecl>(decl)) &&
               tag_decl->getQualifiedNameAsString() ==
                   cast<NamedDecl>(decl)->getQualifiedNameAsString()) {
      // If the sibling is a vardecl or functiondecl and has the same name as
      // the matched node, report the error.
      std::string path =
          misra::libtooling_utils::GetFilename(decl, result.SourceManager);
      int line_number =
          misra::libtooling_utils::GetLine(decl, result.SourceManager);
      ReportError(path, line_number, results_list_);
    } else if (isa<EnumDecl>(decl)) {
      // If the sibling is a enumdecl, check it's enumerators. If there exists a
      // same-name enum type, report the error.
      EnumDecl* enum_decl = cast<EnumDecl>(decl);
      for (auto enum_type_decl : enum_decl->enumerators()) {
        if (enum_type_decl->getNameAsString() == tag_decl->getNameAsString()) {
          std::string path = misra::libtooling_utils::GetFilename(
              enum_decl, result.SourceManager);
          int line_number =
              misra::libtooling_utils::GetLine(enum_decl, result.SourceManager);
          ReportError(path, line_number, results_list_);
        }
      }
    }
  }
}

void Checker::Init(ResultsList* results_list) {
  results_list_ = results_list;
  callback_ = new Callback;
  callback_->Init(results_list, &finder_);
}

MatchFinder* Checker::GetMatchFinder() { return &finder_; }

}  // namespace libtooling
}  // namespace rule_A2_10_6
}  // namespace autosar
