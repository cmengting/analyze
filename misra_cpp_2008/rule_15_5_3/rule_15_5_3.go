/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_5_3

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Reference:
	// https://clang.llvm.org/extra/clang-tidy/index.html
	// https://clang.llvm.org/extra/clang-tidy/checks/list.html
	args := []string{
		"bugprone-exception-escape,bugprone-unhandled-exception-at-new",
		"-header-filter=.*",
	}
	results, err := runner.RunClangTidy(srcdir, args, opts)
	if err != nil {
		return nil, fmt.Errorf("clangtidy err: %v", err)
	}
	for _, res := range results.Results {
		res.ErrorMessage = "不应隐式调用 terminate() 函数"
		res.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_15_5_3
	}

	return results, nil
}
