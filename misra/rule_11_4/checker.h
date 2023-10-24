/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_4_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_4_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_4 {

/*
 * From [misra-c2012-11.4]
 * A conversion should not be performed between a pointer to object and an
 * integer type
 *
 * According to the Amplification and the Examples, this rule applies to:
 * (1) convert a pointer to object to an integer type
 * (2) convert an integer type to a pointer to object
 *
 * Exception:
 * convert null constant pointer with integer type to a pointer to object
 *
 * Procedure:
 * the matcher should match all the cast between a pointer to object and an
 * integer type
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

}  // namespace rule_11_4
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_4_CHECKER_H_
