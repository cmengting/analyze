/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "googlecpp/g1188/libtooling/checker.h"
#include "googlecpp/g1188/libtooling/lib.h"
#include "misra/libtooling_utils/libtooling_utils.h"
#include "misra/proto_util.h"
#include "podman_image/bigmain/suffix_rule.h"

using namespace clang;
using namespace llvm;
using namespace misra::libtooling_utils;
using analyzer::proto::ResultsList;

extern cl::OptionCategory ns_libtooling_checker;
extern cl::opt<std::string> results_path;

namespace {

void ReportError(string path, int line_number, ResultsList* results_list) {
  string error_message =
      "When ordering function parameters, put all input-only parameters before "
      "any output parameters";
  misra::proto_util::AddResultToResultsList(results_list, path, line_number,
                                            error_message);
  LOG(INFO) << absl::StrFormat("%s, path: %s, line: %d", error_message, path,
                               line_number);
}

}  // namespace

namespace googlecpp {
namespace g1188 {
namespace libtooling {
FuncInfo2ParamInfos func_info_2_param_infos;

int g1188(int argc, char** argv) {
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

  googlecpp::g1188::libtooling::Checker checker;
  checker.Init(&all_results);
  int status = tool.run(
      tooling::newFrontendActionFactory(checker.GetMatchFinder()).get());
  LOG(INFO) << "libtooling status: " << status;
  UpdateFuncInfo2ParamInfos(func_info_2_param_infos);
  for (auto const& it : func_info_2_param_infos) {
    FuncInfo const& func_info = it.first;
    ParamInfos const& param_infos = it.second;
    bool previousOutput = false;
    for (ParamInfo const& param_info : param_infos) {
      if (!param_info.is_not_null) continue;
      if (param_info.is_output) {  // Output
        previousOutput = true;
      } else if (previousOutput) {  // Input after output
        ReportError(func_info.path, func_info.line_number, &all_results);
        break;
      }
    }
  }
  if (misra::proto_util::GenerateProtoFile(all_results, results_path).ok()) {
    LOG(INFO) << "g1188 check done";
  }
  return 0;
}
}  // namespace libtooling
}  // namespace g1188
}  // namespace googlecpp

namespace {

podman_image::bigmain::SuffixRule _("googlecpp/g1188",
                                    googlecpp::g1188::libtooling::g1188);

}  // namespace
