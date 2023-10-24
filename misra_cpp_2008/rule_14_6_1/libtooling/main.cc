/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>
#include <llvm/Support/CommandLine.h>

#include "absl/strings/match.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "misra/proto_util.h"
#include "misra_cpp_2008/rule_14_6_1/libtooling/checker.h"
#include "misra_cpp_2008/rule_14_6_1/libtooling/lib.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace misra_cpp_2008 {
namespace rule_14_6_1 {
namespace libtooling {
int rule_14_6_1(int argc, char** argv) {
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
  misra_cpp_2008::rule_14_6_1::libtooling::InfoFillChecker infoFill;
  misra_cpp_2008::rule_14_6_1::libtooling::Checker checker;
  // run infoFill checker first to collect all the needed info
  // for later checking
  infoFill.Init();
  int status = tool.run(
      tooling::newFrontendActionFactory(infoFill.GetMatchFinder()).get());
  LOG(INFO) << "infofill status: " << status;

  checker.Init(&all_results);
  status = tool.run(
      tooling::newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule 14.6.1 check done";
  }

  return 0;
}
}  // namespace libtooling
}  // namespace rule_14_6_1
}  // namespace misra_cpp_2008

namespace {

podman_image::bigmain::SuffixRule _(
    "misra_cpp_2008/rule_14_6_1",
    misra_cpp_2008::rule_14_6_1::libtooling::rule_14_6_1);

}  // namespace
