/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_6_6_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "goto语句引用的标记（label）必须在同一代码块或上级代码块中声明"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_6_6_1
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	cppcheckResults, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_15_3", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Errorf("misra_cpp_2008/rule_6_6_1 Analyze: %v", err)
		return nil, err
	}
	results := generateMisraResult(cppcheckResults)
	return results, nil
}
