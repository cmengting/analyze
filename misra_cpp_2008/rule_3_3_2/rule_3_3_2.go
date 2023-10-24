/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_3_3_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "如果一个函数有内部链接，那么所有的重新声明都必须包括静态（static）存储类说明符"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_3_3_2
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	CppCheckResults, err := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_3_3_2", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Errorf("misra_cpp_2008/rule_3_3_2 Analyze: %v", err)
		return nil, err
	}
	results := generateMisraResult(CppCheckResults)
	return results, nil
}
