/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_2_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func formatReportMessage(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "逻辑运算符&&或||的每个操作数都必须是后缀表达式"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_2_1
	}

	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	result, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_5_2_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	return runner.RemoveDup(formatReportMessage(result)), nil
}
