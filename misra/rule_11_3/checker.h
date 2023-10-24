/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_11_3_CHECKER_H_
#define ANALYZER_MISRA_RULE_11_3_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_11_3 {

/*
 * From [misra-c2012-11.3]
 * A Cast shall not be performed between a pointer to object type and a pointer
 * to a different object type
 *
 * According to the Amplification and the Examples, this rule only applies to:
 * (1) both the source and the destination type in the conversion are pointers
 * (2) the pointer object type should be unqualified
 *
 * Exception:
 * convert pointers to pointers that points to char, signed char and unsigned
 * char is allowed
 *
 * Procedure:
 * the matcher should match all the cast from one pointer type to another
 * pointer type, then we check the PointeeType:
 * - if the Unqualified CanonicalType of PointeeType is different:
 *   - if the destination PointeeType is not char, signed char and unsigned
 *     char, report error
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

}  // namespace rule_11_3
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_11_3_CHECKER_H_
