/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>

#include "absl/strings/match.h"
#include "autosar/rule_A10_3_1/libtooling/checker.h"
#include "autosar/rule_A10_3_1/libtooling/lib.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace autosar {
namespace rule_A10_3_1 {
namespace libtooling {
int rule_A10_3_1(int argc, char** argv) {
  gflags::AllowCommandLineReparsing();
  int gflag_argc = argc;
  int libtooling_argc = argc;
  misra::libtooling_utils::SplitArg(&gflag_argc, &libtooling_argc, argc, argv);
  const char** const_argv = const_cast<const char**>(argv);
  auto expected_parser = CommonOptionsParser::create(
      libtooling_argc, &const_argv[argc - libtooling_argc],
      ns_libtooling_checker);
  gflags::ParseCommandLineFlags(&gflag_argc, &argv, false);
  if (!expected_parser) {
    llvm::errs() << expected_parser.takeError();
    return 1;
  }
  CommonOptionsParser& OptionsParser = expected_parser.get();
  ClangTool tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  analyzer::proto::ResultsList all_results;
  autosar::rule_A10_3_1::libtooling::Checker checker;
  checker.Init(&all_results);
  int status =
      tool.run(newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule_A10_3_1 check done";
  }
  return 0;
}
}  // namespace libtooling
}  // namespace rule_A10_3_1
}  // namespace autosar

namespace {

podman_image::bigmain::SuffixRule _(
    "autosar/rule_A10_3_1", autosar::rule_A10_3_1::libtooling::rule_A10_3_1);
}  // namespace
