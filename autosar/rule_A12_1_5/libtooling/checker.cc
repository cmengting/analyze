/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "autosar/rule_A12_1_5/libtooling/checker.h"

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "misra/libtooling_utils/libtooling_utils.h"

using namespace misra::proto_util;
using namespace clang::ast_matchers;

using analyzer::proto::ResultsList;
using std::string;
using std::vector;

namespace {

void ReportError(const std::string& path, int line_number,
                 ResultsList* results_list) {
  std::string error_message =
      "Common class initialization for non-constant members shall be done by a delegating constructor.";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace autosar {
namespace rule_A12_1_5 {
namespace libtooling {

class Callback : public ast_matchers::MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            ast_matchers::MatchFinder* finder) {
    results_list_ = results_list;
    finder->addMatcher(cxxRecordDecl(isClass(), unless(isImplicit()),
                                     unless(isExpansionInSystemHeader()))
                           .bind("crd"),
                       this);
  }

  void run(const ast_matchers::MatchFinder::MatchResult& result) {
    const CXXRecordDecl* crd = result.Nodes.getNodeAs<CXXRecordDecl>("crd");
    if (crd) {
      vector<vector<const FieldDecl*>> existedCtorInitsSequences{};
      for (const CXXConstructorDecl* ccd : crd->ctors()) {
        if (ccd->isDelegatingConstructor() ||
            ccd->getNumCtorInitializers() == 0)
          continue;
        vector<const FieldDecl*> thisCtorInitsSequence{};
        for (const CXXCtorInitializer* cci : ccd->inits()) {
          if (cci->isInClassMemberInitializer()) continue;
          thisCtorInitsSequence.emplace_back(cci->getMember());
        }
        if (std::find(existedCtorInitsSequences.begin(),
                      existedCtorInitsSequences.end(), thisCtorInitsSequence) ==
            existedCtorInitsSequences.end()) {
          existedCtorInitsSequences.emplace_back(thisCtorInitsSequence);
        } else {
          ReportError(
              misra::libtooling_utils::GetFilename(ccd, result.SourceManager),
              misra::libtooling_utils::GetLine(ccd, result.SourceManager),
              results_list_);
        }
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
}  // namespace rule_A12_1_5
}  // namespace autosar
