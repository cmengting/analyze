/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_21_19_CHECKER_H_
#define ANALYZER_MISRA_RULE_21_19_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_21_19 {

/*
 * From [misra-c2012-21.19]
 * The pointers returned by the Standard Library functions localeconv, getenv,
 * setlocale or strerror shall be const qualified and not editable.
 *
 * Different from  misrac-2012-21_19-ConstPointerReturnChecker.cpp, this matcher
 * provides a simple type check fot part of the rule 21.19:
 * The pointers returned by the Standard Library functions localeconv, getenv,
 * setlocale or strerror shall be assigned to const qualified variables.
 *
 * Exception:
 * The return value can be casted into void.(ignored)
 *
 * Procedure:
 * (1) the matcher should match all the cast from a call to these functions:
 *  - if the Destination type is void, ignore;
 *  - if the Destination type is a pointer type, and the pointee type is not
 *    const qualified, report error.
 *  - integer type conversion for const is not supported by AST matcher, ignored
 * (2) if there is a call to these functions without cast in it's parent expr,
 *    then it's assigned to a non-const type. report error.
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

}  // namespace rule_21_19
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_21_19_CHECKER_H_
