/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_16_1_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_16_1_1", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range results.Results {
		r.ErrorMessage = "defined预处理操作符只能使用两种标准形式之一"
		r.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_16_1_1
	}
	return results, nil
}
