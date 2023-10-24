/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_8_5_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "在枚举数列表中，= 构造不应用于显式初始化除第一个以外的成员，除非所有项目都显式初始化"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_8_5_3
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results1, _ := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_8_5_3", checker_integration.Cppcheck_STU, opts)
	results := generateMisraResult(results1)
	return results, nil
}
