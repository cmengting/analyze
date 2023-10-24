/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_8_5_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "在数组和结构的非零初始化中，应使用大括号来指示和匹配结构"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_8_5_2
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_8_5_2", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	return generateMisraResult(results), nil
}
