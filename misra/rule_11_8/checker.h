/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_8_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_8_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_8 {

/*
 * From [misra-c2012-11.8]
 * A cast shall not remove any const or volatile qualification from the type
 * pointed to by a pointer
 *
 * this rule should only applies to:
 * (1) both the source and the destination type in the conversion are pointers
 *
 * Note:
 * the qualification is for the type pointed to, not the pointer itself.
 *
 * Procedure:
 * the matcher should match all the cast from one pointer type to another
 * pointer type, then we check the PointeeType:
 * - if source PointeeType has const qualification, destination has not, report
 * error
 * - if source PointeeType has volatile qualification, destination has not,
 * report error
 *
 */
class CastCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list);

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  CastCallback* callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_11_8
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_8_CHECKER_H_
