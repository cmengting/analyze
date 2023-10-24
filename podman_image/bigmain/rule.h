/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#ifndef ANALYZER_PODMAN_IMAGE_BIGMAIN_RULE_H_
#define ANALYZER_PODMAN_IMAGE_BIGMAIN_RULE_H_

#include <vector>

namespace podman_image {
namespace bigmain {

class Rule {
 public:
  static std::vector<Rule*>& GetAllRules() {
    static std::vector<Rule*>* all_rules = new std::vector<Rule*>();
    return *all_rules;
  }

  Rule() { GetAllRules().push_back(this); }

  // Returns true if this rule is handled
  virtual bool Entrypoint(int argc, char** argv, int* return_value) = 0;
};

}  // namespace bigmain
}  // namespace podman_image

#endif
