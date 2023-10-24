/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1200/libtooling/checker.h"

#include <glog/logging.h>

#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang;
using namespace misra::proto_util;

namespace googlecpp {
namespace g1200 {
namespace libtooling {

void Check::Init(analyzer::proto::ResultsList* results_list,
                 SourceManager* source_manager) {
  results_list_ = results_list;
  source_manager_ = source_manager;
}

void Check::MacroDefined(const Token& macro_name_tok,
                         const MacroDirective* md) {
  if (!md->isDefined()) return;  // #define
  const MacroInfo* macro_info = md->getMacroInfo();
  if (macro_info->tokens_empty()) return;  // #define HEADER_GUARD shoule be OK

  const auto macro_loc = macro_info->getDefinitionLoc();
  if (source_manager_->isInSystemHeader(macro_loc)) return;
  if (source_manager_->isInSystemMacro(macro_loc)) return;

  string path =
      misra::libtooling_utils::GetRealFilename(macro_loc, source_manager_);
  if (path.length() < 2) return;
  if (path.substr(path.length() - 2, 2) != ".h")
    return;  // ensure in header file

  int line = misra::libtooling_utils::GetRealLine(macro_loc, source_manager_);
  string name = macro_name_tok.getIdentifierInfo()->getName().str();

  string error_message =
      "Avoid defining macros, especially in headers; prefer inline functions, enums, and const variables";
  AddResultToResultsList(results_list_, path, line, error_message);
  LOG(INFO) << absl::StrFormat("%s, name: %s, path: %s, line: %d",
                               error_message, name, path, line);
}

bool Action::BeginSourceFileAction(CompilerInstance& ci) {
  std::unique_ptr<Check> callback(new Check());
  callback->Init(results_list_, &ci.getSourceManager());
  Preprocessor& pp = ci.getPreprocessor();
  pp.addPPCallbacks(std::move(callback));
  return true;
}

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
}

}  // namespace libtooling
}  // namespace g1200
}  // namespace googlecpp
