/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_PODMAN_IMAGE_BIGMAIN_SUFFIX_RULE_H_
#define ANALYZER_PODMAN_IMAGE_BIGMAIN_SUFFIX_RULE_H_

#include <string>

#include "podman_image/bigmain/rule.h"

namespace podman_image {
namespace bigmain {

class SuffixRule : Rule {
 public:
  typedef int (*Fn)(int, char**);

  SuffixRule(std::string suffix, Fn fn) : suffix_(suffix), fn_(fn) {}

  bool Entrypoint(int argc, char** argv, int* return_value) override;

 private:
  std::string suffix_;
  Fn fn_;
};

}  // namespace bigmain
}  // namespace podman_image

#endif
