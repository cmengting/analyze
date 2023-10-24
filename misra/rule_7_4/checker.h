/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_MISRA_RULE_7_4_CHECKER_H_
#define ANALYZER_MISRA_RULE_7_4_CHECKER_H_

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "misra/proto_util.h"

namespace misra {
namespace rule_7_4 {

/*
 * From [misra-c2012-7.4]
 * A string literal shall be assigned to an object with pointer to
 * const-qualified type
 *
 * According to the Amplification and the Examples, this rule applies to:
 * (1) directly modify a string literal
 * (2) assgin a string literal to an object with improper type
 * These situations can be detected by checking each caseExpr from a string
 * literal to another object.
 *
 * However, there is a problem:
 * const char* a = "123";
 *
 * In the AST given by clang for the good case above, there are two casts:
 * (1) An ArrayToPointerDecay cast from char[4] to char *
 * (2) A NoOp cast from char * to const char *
 *
 * But in the bad case:
 * char* a = "123";
 *
 * There is only one cast: An ArrayToPointerDecay cast from char[4] to char *
 *
 * In most cases, a string literal is casted at least once. And we should focus
 * on the other casts.
 *
 * A map<std::string, StringLiteralInfo> count_str_literal_cast is used. Its key
 * is the location of the string literal. Its value a struct which contains
 * the counter, path and line number of the string literal.
 *
 * The counter increases by 1 for each cast.
 * The counters decreases by 2 if the following conditions are met:
 *  (1) the destination type is const-qualified
 *  (2) the source and estination have the same type which they point to (char
 *      or wchar type)
 *
 * Procedure:
 * the matcher should match all the casts from a string literal to some object
 *  (1) remove restrict/volatile qualifer for destination type
 *  (2) check if the location of the string literal exists in
 *      count_str_literal_cast:
 *      yes: add the counter by 1
 *      no:  initialize the counter with 1
 *  (3) if the cast meets the condtions above. Substract the counter by 2
 *  (4) after all casts are matched, find out the string literals with
 *      greater-than-zero counter and they are the string literals which violate
 *      rule 7.4
 *
 */

typedef struct {
  int count_;
  int line_;
  std::string path_;
} StringLiteralInfo;

class CastCallback;

class Checker {
 public:
  void Init(analyzer::proto::ResultsList* results_list,
            std::map<std::string, StringLiteralInfo>* count_str_literal_cast);
  void FindInvalidStringLiteralAssignment();

  clang::ast_matchers::MatchFinder* GetMatchFinder() { return &finder_; }

 private:
  std::map<std::string, StringLiteralInfo>* count_str_literal_cast_;
  CastCallback* callback_;
  clang::ast_matchers::MatchFinder finder_;
  analyzer::proto::ResultsList* results_list_;
};

}  // namespace rule_7_4
}  // namespace misra

#endif  // ANALYZER_MISRA_RULE_7_4_CHECKER_H_
