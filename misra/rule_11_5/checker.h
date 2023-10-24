/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_5_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_5_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_5 {

/*
 * From [misra-c2012-11.5]
 * A conversion should not be performed from pointer to void into pointer to
 * object
 *
 * According to the Examples, this rule only applies to:
 * (1) the source type in the conversion is pointer to void
 * (2) the destination type in the conversion is pointer to non-void
 *
 * Exception:
 * A null pointer constant that has type pointer to void may be converted into
 * pointer to object
 *
 * Procedure:
 * the matcher should match all the cast from void pointer type to non-void
 * pointer type, then we check the destination isNullPointerConstant:
 *  - if the destination isNullPointerConstant is NotNull, report error
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

}  // namespace rule_11_5
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_5_CHECKER_H_
