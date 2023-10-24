/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "misra/rule_8_7/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace std;
using namespace clang;
using namespace clang::ast_matchers;
using namespace misra::proto_util;
using analyzer::proto::ResultsList;

namespace {

string GetDeclName(const NamedDecl* decl) { return decl->getNameAsString(); }

void ReportSingleExternError(string name, string loc, int line_number,
                             ResultsList* results_list) {
  std::string error_message = absl::StrFormat(
      "[C0508][misra-c2012-8.7]: violation of misra-c2012-8.7\n"
      "Extern function or variable is only called at one translation unit\n"
      "function name: %s\n"
      "location: %s",
      name, loc);
  analyzer::proto::Result* pb_result =
      AddResultToResultsList(results_list, loc, line_number, error_message);
  pb_result->set_error_kind(
      analyzer::proto::Result_ErrorKind_MISRA_C_2012_RULE_8_7);
  pb_result->set_name(name);
  pb_result->set_loc(loc);
  LOG(INFO) << error_message;
}

}  // namespace

namespace misra {
namespace rule_8_7 {

class ExternalVDCallback : public MatchFinder::MatchCallback {
 public:
  void Init(
      ResultsList* results_list, MatchFinder* finder,
      unordered_map<string, vector<pair<string, int>>>* vd_name_locations_) {
    results_list_ = results_list;
    finder->addMatcher(declRefExpr().bind("dre"), this);
    name_locations_ = vd_name_locations_;
  }

  void run(const MatchFinder::MatchResult& result) override {
    const DeclRefExpr* dre = result.Nodes.getNodeAs<DeclRefExpr>("dre");
    ASTContext* context = result.Context;
    if (libtooling_utils::IsInSystemHeader(dre, context)) {
      return;
    }
    const ValueDecl* vd = dre->getDecl();
    if (libtooling_utils::IsInSystemHeader(vd, context)) {
      return;
    }
    if (!vd->hasExternalFormalLinkage()) {
      return;
    }
    string path = libtooling_utils::GetFilename(dre, result.SourceManager);
    int line_number = libtooling_utils::GetLine(dre, result.SourceManager);

    auto name = GetDeclName(vd);
    // get the name location key-value pair we stored before
    auto it = name_locations_->find(name);
    if (it != name_locations_->end()) {
      // if the existing key-value pair has the same path as the current
      // function/value call then we should not add current ce to
      // name_locations_
      for (auto path_pair : it->second) {
        if (path_pair.first == path) {
          return;
        }
      }
      it->second.push_back(make_pair(path, line_number));
    } else {
      name_locations_->emplace(make_pair(
          name, vector<pair<string, int>>{make_pair(path, line_number)}));
    }
  }

 private:
  ResultsList* results_list_;
  unordered_map<string, vector<pair<string, int>>>* name_locations_;
};

void Checker::Run() {
  for (auto& name_loc : vd_name_locations_) {
    if (name_loc.second.size() == 1) {
      ReportSingleExternError(name_loc.first, name_loc.second[0].first,
                              name_loc.second[0].second, results_list_);
    }
  }
}

void Checker::Init(analyzer::proto::ResultsList* results_list) {
  results_list_ = results_list;
  vd_callback_ = new ExternalVDCallback;
  vd_callback_->Init(results_list_, &finder_, &vd_name_locations_);
}

}  // namespace rule_8_7
}  // namespace misra
