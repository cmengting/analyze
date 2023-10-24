/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>

#include "autosar/rule_A2_3_1/libtooling/checker.h"
#include "autosar/rule_A2_3_1/libtooling/lib.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "misra/proto_util.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace autosar {
namespace rule_A2_3_1 {
namespace libtooling {
int rule_A2_3_1(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::AllowCommandLineReparsing();
  int gflag_argc = argc;
  int libtooling_argc = argc;
  misra::libtooling_utils::SplitArg(&gflag_argc, &libtooling_argc, argc, argv);
  const char** const_argv = const_cast<const char**>(argv);
  gflags::ParseCommandLineFlags(&gflag_argc, &argv, false);

  auto ep = tooling::CommonOptionsParser::create(
      libtooling_argc, &const_argv[argc - libtooling_argc],
      ns_libtooling_checker);
  if (!ep) {
    llvm::errs() << ep.takeError();
    return 1;
  }
  tooling::CommonOptionsParser& op = ep.get();
  tooling::ClangTool tool(op.getCompilations(), op.getSourcePathList());
  analyzer::proto::ResultsList all_results;

  autosar::rule_A2_3_1::libtooling::Checker checker1;
  checker1.Init(&all_results);
  int status1 = tool.run(
      tooling::newFrontendActionFactory(checker1.GetMatchFinder()).get());
  autosar::rule_A2_3_1::libtooling::CommentChecker checker2(&all_results);
  tool.appendArgumentsAdjuster(
      clang::tooling::getInsertArgumentAdjuster("-fparse-all-comments"));
  int status2 = tool.run(&checker2);
  int result_status = (status1 | status2);
  LOG(INFO) << "libtooling status: "
            << (result_status != 0 ? result_status : 0);
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule_A2_3_1 check done";
  }
  return 0;
}
}  // namespace libtooling
}  // namespace rule_A2_3_1
}  // namespace autosar

namespace {

podman_image::bigmain::SuffixRule _(
    "autosar/rule_A2_3_1", autosar::rule_A2_3_1::libtooling::rule_A2_3_1);

}  // namespace
