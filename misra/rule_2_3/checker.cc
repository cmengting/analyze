/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_2_3/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace misra {
namespace rule_2_3 {

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  finder_.addMatcher(typedefDecl().bind("typedef_decl"), this);
}

void Checker::run(const MatchFinder::MatchResult& result) {
  clang::ASTContext* context = result.Context;
  const clang::TypedefDecl* typedef_decl =
      result.Nodes.getNodeAs<clang::TypedefDecl>("typedef_decl");

  // If a typedefed type has been used, it will have a referenced attribute to
  // show that.
  if (typedef_decl->isReferenced()) {
    return;
  }
  clang::FullSourceLoc location =
      context->getFullLoc(typedef_decl->getBeginLoc());
  std::string path =
      libtooling_utils::GetFilename(typedef_decl, result.SourceManager);
  int line_number =
      libtooling_utils::GetLine(typedef_decl, result.SourceManager);

  // For isValid(): we need to filter out the typedef
  // which gives invalid location. As for why do we
  // have some typedef in invalid location
  // please see
  // https://b.corp.naive.systems:9443/projects/misra-c-2012/wiki/_why_should_we_use_isValid()_

  // For system header: If we include some system header file into .c file,
  // there are some unused type declarations in system header,
  // it will be reported. To prevent this, we need to
  // decide that whether current type declaration is in system header or not.
  if (location.isValid() && !location.isInSystemHeader()) {
    std::string error_message = absl::StrFormat(
        "[C2005][misra-c2012-2.3]: violation of misra-c2012-2.3\n"
        "unused typedef name: %s",
        typedef_decl->getNameAsString());
    analyzer::proto::Result* pb_result =
        AddResultToResultsList(results_list_, path, line_number, error_message);
    pb_result->set_error_kind(
        analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_2_3);
    pb_result->set_typedef_decl_name(typedef_decl->getNameAsString());
    LOG(INFO) << error_message;
  }
}

}  // namespace rule_2_3
}  // namespace misra
