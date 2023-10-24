/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_2_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_2_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_2 {

/*
 * From [misra-c2012-11.3]
 * Conversions shall not be performed between a pointer to an incomplete
 * type and any other type
 *
 * According to the Amplification and the Exception:
 * (1) a pointer to an incomplete type here is not void type
 * (2) a null pointer constant may be converted into a pointer to an incomplete
 * type
 * (3) a pointer to an incomplete type may be converted into void
 * (4) an incomplete type here should be unqualified
 *
 * Procedure:
 * the matcher should match all the cast from or into a pointer type:
 * (this is because we cannot match incomplete type directly.)
 * - if this is a cast into a pointer:
 *   - if this cast is not into a pointer to an incomplete type, return
 *   - if this cast is from null, return
 *   - if this cast is into a pointer to void, return
 * - if this is a cast from a pointer to an incomplete type:
 *   - if this cast is not from a pointer to an incomplete type, return
 *   - if this cast is into void type, return
 *   - if this cast is from a pointer to void, return
 * - if the unqualified pointer object is the same, return
 * - report error
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

}  // namespace rule_11_2
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_2_CHECKER_H_
