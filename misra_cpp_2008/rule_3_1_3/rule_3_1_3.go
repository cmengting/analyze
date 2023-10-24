/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_3_1_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func formatReportMessage(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "声明一个数组时，必须显式说明其大小，或通过初始化隐式定义其大小"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_3_1_3
	}

	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_3_1_3", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	return formatReportMessage(results), nil
}
