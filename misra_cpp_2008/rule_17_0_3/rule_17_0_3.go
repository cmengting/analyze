/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_0_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	rs, err := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_17_0_3", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range rs.Results {
		r.ErrorMessage = "标准库函数的名称不应被覆盖"
		r.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_17_0_3
	}
	return rs, nil
}
