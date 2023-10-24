/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_1_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_1_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_1 {

/*
 * From [misra-c2012-11.1]
 * Conversions shall not be performed between a pointer to a function
 * and any other type
 *
 * Amplification:
 * a pointer to a function shall only be converted into or from a pointer to a
 * function with a compatible type. In Exception, we can see a compatible type
 * is:
 * - null pointer constant (Exception 1)
 * - void (Exception 2)
 * - that function type (Exception 3)
 *
 * Procedure:
 * - match all casts from or into a function pointer type, as
 *   'pointsTo(parenType(innerType(functionType())))', after that:
 *   - use 'isNullPointerConstant' for Exception 1
 *   - use 'isVoidType' for Exception 2
 *   - use 'isFunctionType' and 'getPointeeType' for Exception 3
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

}  // namespace rule_11_1
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_1_CHECKER_H_
