/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "googlecpp/g1151/libtooling/checker.h"

#include <glog/logging.h>

#include <fstream>
#include <ios>
#include <string>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using std::string;
using namespace clang;
using namespace misra::proto_util;
using namespace misra::libtooling_utils;

namespace googlecpp {
namespace g1151 {
namespace libtooling {

void Check::Init(analyzer::proto::ResultsList* results_list,
                 SourceManager* source_manager,
                 const std::string& optional_info_file) {
  results_list_ = results_list;
  source_manager_ = source_manager;
  ofs.open(optional_info_file, std::ios::app);
}

void Check::InclusionDirective(SourceLocation HashLoc, const Token& IncludeTok,
                               StringRef FileName, bool IsAngled,
                               CharSourceRange FilenameRange,
                               Optional<FileEntryRef> File,
                               StringRef SearchPath, StringRef RelativePath,
                               const Module* Imported,
                               SrcMgr::CharacteristicKind FileType) {
  if (IsAngled) return;
  if (source_manager_->isInSystemHeader(HashLoc)) return;
  if (source_manager_->isInSystemMacro(HashLoc)) return;

  string path = GetRealFilename(HashLoc, source_manager_);
  int line = GetRealLine(HashLoc, source_manager_);
  StringRef pathRef(path);

  if (HasHeaderSuffix(FileName)) {
    // Add what header files it refers to into logs.
    ofs << path << " " << FileName.str() << "\n";
  }

  // We only check inclusion not in header files
  // e.g. it's acceptable for .h file to include .inc file
  if (HasHeaderSuffix(pathRef)) return;

  // Then we report any inclusion that not ends in .h
  if (!HasHeaderSuffix(FileName)) {
    string error_message =
        "Header files should be self-contained (compile on their own) and end in .h";
    AddResultToResultsList(results_list_, path, line, error_message);
    LOG(INFO) << absl::StrFormat("%s, name: %s, path: %s, line: %d",
                                 error_message, FileName.str(), path, line);
  }
}

bool Action::BeginSourceFileAction(CompilerInstance& ci) {
  std::unique_ptr<Check> callback(new Check());
  callback->Init(results_list_, &ci.getSourceManager(), optional_info_file_);
  Preprocessor& pp = ci.getPreprocessor();
  pp.addPPCallbacks(std::move(callback));
  return true;
}

void Checker::Init(analyzer::proto::ResultsList* results_list,
                   const std::string& optional_info_file) {
  results_list_ = results_list;
  optional_info_file_ = optional_info_file;
}

}  // namespace libtooling
}  // namespace g1151
}  // namespace googlecpp
