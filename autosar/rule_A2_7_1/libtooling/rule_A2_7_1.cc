/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>

#include "absl/strings/match.h"
#include "autosar/rule_A2_7_1/libtooling/checker.h"
#include "autosar/rule_A2_7_1/libtooling/lib.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "misra/proto_util.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang::tooling;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace autosar {
namespace rule_A2_7_1 {
namespace libtooling {
int rule_A2_7_1(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
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
  CommonOptionsParser& options_parser = expected_parser.get();
  vector<string> path_list = options_parser.getSourcePathList();
  if (path_list.size() != 1) {
    llvm::errs() << "The number of filepath is not equal to 1";
    return 1;
  }
  tooling::ClangTool tool(
      options_parser.getCompilations(),
      misra::libtooling_utils::GetCTUSourceFile(path_list[0]));
  analyzer::proto::ResultsList all_results;
  autosar::rule_A2_7_1::libtooling::CommentChecker checker(&all_results);

  tool.appendArgumentsAdjuster(
      getInsertArgumentAdjuster("-fparse-all-comments"));
  int status = tool.run(&checker);
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule_A2_7_1 check done";
  }
  return 0;
}
}  // namespace libtooling
}  // namespace rule_A2_7_1
}  // namespace autosar

namespace {

podman_image::bigmain::SuffixRule _(
    "autosar/rule_A2_7_1", autosar::rule_A2_7_1::libtooling::rule_A2_7_1);

}  // namespace
