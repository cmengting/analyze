/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>
#include <llvm/Support/JSON.h>

#include "absl/strings/match.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "misra/rule_1_1/checker.h"
#include "misra/rule_1_1/lib.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang::tooling;
using namespace llvm;

extern cl::OptionCategory ns_libtooling_checker;
static cl::extrahelp common_help(CommonOptionsParser::HelpMessage);
extern cl::opt<std::string> results_path;
extern cl::opt<int> struct_member_limit;
extern cl::opt<int> function_parm_limit;
extern cl::opt<int> function_arg_limit;
extern cl::opt<int> nested_record_limit;
extern cl::opt<int> nested_expr_limit;
extern cl::opt<int> switch_case_limit;
extern cl::opt<int> enum_constant_limit;
extern cl::opt<int> string_char_limit;
extern cl::opt<int> extern_id_limit;

namespace misra {
namespace rule_1_1 {
int rule_1_1(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::AllowCommandLineReparsing();
  int gflag_argc = argc;
  int libtooling_argc = argc;
  misra::libtooling_utils::SplitArg(&gflag_argc, &libtooling_argc, argc, argv);
  const char** const_argv = const_cast<const char**>(argv);
  gflags::ParseCommandLineFlags(&gflag_argc, &argv, false);

  auto expected_parser = CommonOptionsParser::create(
      libtooling_argc, &const_argv[argc - libtooling_argc],
      ns_libtooling_checker);
  if (!expected_parser) {
    llvm::errs() << expected_parser.takeError();
    return 1;
  }
  CommonOptionsParser& options_parser = expected_parser.get();
  ClangTool tool(options_parser.getCompilations(),
                 options_parser.getSourcePathList());
  analyzer::proto::ResultsList all_results;
  misra::rule_1_1::Checker checker;
  LimitList limits{struct_member_limit, function_parm_limit, function_arg_limit,
                   nested_record_limit, nested_expr_limit,   switch_case_limit,
                   enum_constant_limit, string_char_limit,   extern_id_limit};
  checker.Init(limits, &all_results);
  int status =
      tool.run(newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "rule 1.1 check done";
  }
  return 0;
}
}  // namespace rule_1_1
}  // namespace misra

namespace {

podman_image::bigmain::SuffixRule _("misra/rule_1_1",
                                    misra::rule_1_1::rule_1_1);

}  // namespace
