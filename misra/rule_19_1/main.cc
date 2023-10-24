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
#include "misra/rule_19_1/checker.h"
#include "misra/rule_19_1/lib.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace misra {
namespace rule_19_1 {
int rule_19_1(int argc, char** argv) {
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
  misra::rule_19_1::Checker checker;
  checker.Init(&all_results);
  int status = tool.run(
      tooling::newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule 19.1 check done";
  }
  return 0;
}
}  // namespace rule_19_1
}  // namespace misra

namespace {

podman_image::bigmain::SuffixRule _("misra/rule_19_1",
                                    misra::rule_19_1::rule_19_1);

}  // namespace
