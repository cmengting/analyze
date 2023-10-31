/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_5_1/checker.h"

#include <glog/logging.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace std;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

bool ContainNonAsciiChar(std::string s) {
  for (unsigned char c : s) {
    if (c >= 128) {
      return true;
    }
  }
  return false;
}

string CalcDistinctName(string name, int prefix_length, bool case_sensitive) {
  name = name.substr(0, prefix_length);
  if (!case_sensitive) {
    for (char& c : name) {
      c = tolower(c);
    }
  }
  return name;
}

void ReportNonAsciiError(string kind, string name, string path, int line_number,
                         ResultsList* results_list) {
  string error_message = absl::StrFormat(
      "[C1109][misra-c2012-5.1]: contain non-ASCII characters\n"
      "%s: %s",
      kind, name);
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, path, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_5_1_NON_ASCII_ERROR);
  pb_result->set_kind(kind);
  pb_result->set_name(name);
  LOG(INFO) << error_message;
}

void ReportDistinctError(string kind, string name, string loc, string other_loc,
                         string path, int line_number,
                         ResultsList* results_list) {
  string error_message = absl::StrFormat(
      "[C1109][misra-c2012-5.1]: violation of misra-c2012-5.1\n"
      "%s: %s\n"
      "First identifier location: %s\n"
      "Duplicated identifier location: %s",
      kind, name, loc, other_loc);
  vector<string> locations{loc, other_loc};
  analyzer::proto::Result* pb_result = AddMultipleLocationsResultToResultsList(
      results_list, path, line_number, error_message, locations);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_5_1_DISTINCT_ERROR);
  pb_result->set_kind(kind);
  pb_result->set_name(name);
  pb_result->set_loc(loc);
  pb_result->set_other_loc(other_loc);
  LOG(INFO) << error_message;
}

}  // namespace

namespace misra {
namespace rule_5_1 {

class ExternalVDCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int prefix_length, bool case_sensitive,
            unordered_map<string, string>* name_locations,
            ResultsList* results_list, MatchFinder* finder) {
    prefix_length_ = prefix_length;
    case_sensitive_ = case_sensitive;
    name_locations_ = name_locations;
    results_list_ = results_list;
    finder->addMatcher(varDecl().bind("vd"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const VarDecl* vd = result.Nodes.getNodeAs<VarDecl>("vd");
    if (!vd->hasExternalFormalLinkage() || vd->hasExternalStorage() ||
        vd->isWeak()) {
      return;
    }
    // skip system header
    if (libtooling_utils::IsInSystemHeader(vd, result.Context)) {
      return;
    }
    string name = vd->getNameAsString();
    string path = libtooling_utils::GetFilename(vd, result.SourceManager);
    int line_number = libtooling_utils::GetLine(vd, result.SourceManager);
    string loc = libtooling_utils::GetLocation(vd, result.SourceManager);
    if (ContainNonAsciiChar(name)) {
      ReportNonAsciiError("Variable", name, path, line_number, results_list_);
      return;
    }
    string distinct_name =
        CalcDistinctName(name, prefix_length_, case_sensitive_);
    auto it = name_locations_->find(distinct_name);
    if (it == name_locations_->end()) {
      name_locations_->emplace(distinct_name, loc);
    } else if (loc != it->second) {
      ReportDistinctError("Variable", name, loc, it->second, path, line_number,
                          results_list_);
    }
  }

 private:
  int prefix_length_;
  bool case_sensitive_;
  unordered_map<string, string>* name_locations_;
  ResultsList* results_list_;
};

class ExternalFDCallback : public MatchFinder::MatchCallback {
 public:
  void Init(int prefix_length, bool case_sensitive,
            unordered_map<string, string>* name_locations,
            ResultsList* results_list, MatchFinder* finder) {
    prefix_length_ = prefix_length;
    case_sensitive_ = case_sensitive;
    name_locations_ = name_locations;
    results_list_ = results_list;
    finder->addMatcher(functionDecl().bind("fd"), this);
  }

  void run(const MatchFinder::MatchResult& result) override {
    const FunctionDecl* fd = result.Nodes.getNodeAs<FunctionDecl>("fd");
    if (!fd->hasExternalFormalLinkage() || fd->isWeak()) {
      return;
    }
    // skip system header
    if (libtooling_utils::IsInSystemHeader(fd, result.Context)) {
      return;
    }
    // skip function declarations in header
    if (!fd->doesThisDeclarationHaveABody()) {
      return;
    }
    string name = fd->getNameAsString();
    string path = libtooling_utils::GetFilename(fd, result.SourceManager);
    int line_number = libtooling_utils::GetLine(fd, result.SourceManager);
    string loc = libtooling_utils::GetLocation(fd, result.SourceManager);
    if (ContainNonAsciiChar(name)) {
      ReportNonAsciiError("Function", name, path, line_number, results_list_);
      return;
    }
    string distinct_name =
        CalcDistinctName(name, prefix_length_, case_sensitive_);
    auto it = name_locations_->find(distinct_name);
    if (it == name_locations_->end()) {
      name_locations_->emplace(distinct_name, loc);
    } else if (loc != it->second) {
      ReportDistinctError("Function", name, loc, it->second, path, line_number,
                          results_list_);
    }
  }

 private:
  int prefix_length_;
  bool case_sensitive_;
  unordered_map<string, string>* name_locations_;
  ResultsList* results_list_;
};

void Checker::Init(int prefix_length, bool case_sensitive,
                   ResultsList* results_list) {
  // TODO: fix leaks when necessary
  results_list_ = results_list;
  external_vd_callback_ = new ExternalVDCallback;
  external_vd_callback_->Init(prefix_length, case_sensitive, &name_locations_,
                              results_list_, &finder_);
  external_fd_callback_ = new ExternalFDCallback;
  external_fd_callback_->Init(prefix_length, case_sensitive, &name_locations_,
                              results_list_, &finder_);
}

}  // namespace rule_5_1
}  // namespace misra
