/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_AUTOSAR_A1_1_1_LIBTOOLING_CHECKER_H_
#define ANALYZER_AUTOSAR_A1_1_1_LIBTOOLING_CHECKER_H_

#include <clang/Basic/Diagnostic.h>

#include "misra/proto_util.h"

namespace autosar {
namespace rule_A1_1_1 {
namespace libtooling {

class Checker : public clang::DiagnosticConsumer {
 public:
  void Init(analyzer::proto::ResultsList* results_list);
  void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                        const clang::Diagnostic& d) override;

 private:
  analyzer::proto::ResultsList* results_list_;
};
}  // namespace libtooling
}  // namespace rule_A1_1_1
}  // namespace autosar
#endif  // ANALYZER_AUTOSAR_rule_A1_1_1_LIBTOOLING_CHECKER_H_
