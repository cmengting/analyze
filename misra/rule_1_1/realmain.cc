/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "libtooling_includes/cmd_options.h"
#include "misra/rule_1_1/lib.h"
int main(int argc, char** argv) {
  return misra::rule_1_1::rule_1_1(argc, argv);
}
