/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_7_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_7_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_7 {

/*
 * From [misra-c2012-11.7]
 * A cast shall not be performed between pointer to object and a non-interger
 * arithmetic type
 *
 * Amplification:
 * a non-interger arithmetic type is:
 * - boolean
 * - character
 * - enum
 * - floating
 *
 * Procedure:
 * This is similar to 11.6. In 11.6, 'isInteger()' can match all the boolean,
 * character and enum type, but here we need to separate them from basic
 * interger type.
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

}  // namespace rule_11_7
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_7_CHECKER_H_
