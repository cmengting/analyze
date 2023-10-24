/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include <glog/logging.h>

#include "absl/strings/str_format.h"
#include "libtooling_includes/cmd_options.h"
#include "podman_image/bigmain/rule.h"

using Rule = podman_image::bigmain::Rule;

int main(int argc, char** argv) {
  std::vector<Rule*>& all_rules = Rule::GetAllRules();
  for (Rule* rule : all_rules) {
    int r;
    if (rule->Entrypoint(argc, argv, &r)) return r;
  }
  google::InitGoogleLogging(argv[0]);
  LOG(ERROR) << absl::StrFormat("rules size: %d, incompatible argv[0]: %s",
                                all_rules.size(), argv[0]);
  return -1;
}
