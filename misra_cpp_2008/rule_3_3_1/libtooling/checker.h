/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_CPP_2008_RULE_3_3_1_LIBTOOLING_CHECKER_H_
#define ANALYZER_MISRA_CPP_2008_RULE_3_3_1_LIBTOOLING_CHECKER_H_

#include <string.h>

#include <unordered_map>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "misra/proto_util.h"

using namespace clang;

namespace misra_cpp_2008 {
namespace rule_3_3_1 {
namespace libtooling {

using MatchFinder = clang::ast_matchers::MatchFinder;

enum DeclState { DECLARED_IN_HEADER, DECLARED_WITH_EXETRNAL_LINKAGE };

class Checker : public MatchFinder::MatchCallback {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  MatchFinder* GetMatchFinder() { return &finder_; }

  void run(const MatchFinder::MatchResult& result) override;

  void setDeclsInHeader(NamedDecl* decl);

  void checkDeclsInSource(NamedDecl* decl, SourceManager* sourceManager);

  bool isDefinition(NamedDecl* decl);

  bool hasDefinition(NamedDecl* decl);

  NamedDecl* getDefinition(NamedDecl* decl);

  bool isMainFunc(NamedDecl* decl);

  bool isInHeader(NamedDecl* decl, SourceManager* sourceManager);

  void reportInValidDecl();

 private:
  MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
  std::unordered_map<NamedDecl*, DeclState> decl_state_;
  std::unordered_map<NamedDecl*, std::pair<std::string, int>> decl_record_;
};

}  // namespace libtooling
}  // namespace rule_3_3_1
}  // namespace misra_cpp_2008
#endif  // ANALYZER_MISRA_CPP_2008_RULE_3_3_1_LIBTOOLING_CHECKER_H_
