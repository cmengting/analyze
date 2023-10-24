/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_0_9

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func formatReportMessage(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "显式的整型转换不得改变c值表达式的底层类型的符号性"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_0_9
	}

	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	result, err := runner.RunLibtoolingWithExtraArgs(srcdir, "misra_cpp_2008/rule_5_0_8", "--rule-name=rule-5-0-9", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	return formatReportMessage(result), nil
}
