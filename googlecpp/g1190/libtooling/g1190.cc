/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>

#include "googlecpp/g1190/libtooling/checker.h"
#include "googlecpp/g1190/libtooling/lib.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang::tooling;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
static cl::extrahelp common_help(CommonOptionsParser::HelpMessage);
extern cl::opt<std::string> results_path;

namespace googlecpp {
namespace g1190 {
namespace libtooling {
int g1190(int argc, char** argv) {
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
  ClangTool tool(options_parser.getCompilations(),
                 options_parser.getSourcePathList());
  googlecpp::g1190::libtooling::ASTChecker checker;
  analyzer::proto::ResultsList all_results;

  checker.Init(&all_results);
  int status =
      tool.run(newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "g1190 check done";
  }
  return 0;
}
}  // namespace libtooling
}  // namespace g1190
}  // namespace googlecpp

namespace {

podman_image::bigmain::SuffixRule _("googlecpp/g1190",
                                    googlecpp::g1190::libtooling::g1190);

}  // namespace
