/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_0_6

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_5_0_6", checker_integration.Libtooling_STU, opts)
	// merge two types of cast error messages
	newResults := &pb.ResultsList{}
	for _, result := range results.Results {
		if runner.HasTargetErrorMessageFragment(result, "隐式的浮点转换不应使底层类型的大小变小") || runner.HasTargetErrorMessageFragment(result, "隐式的整数转换不应使底层类型的大小变小") {
			result.ErrorMessage = "隐式的整数或浮点转换不应使底层类型的大小变小"
			newResults.Results = append(newResults.Results, result)
		}
	}
	return newResults, err
}
