/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "podman_image/bigmain/suffix_rule.h"

#include "absl/strings/match.h"

namespace podman_image {
namespace bigmain {

bool SuffixRule::Entrypoint(int argc, char** argv, int* return_value) {
  if (!absl::EndsWith(argv[0], suffix_)) return false;
  *return_value = fn_(argc, argv);
  return true;
}

}  // namespace bigmain
}  // namespace podman_image
