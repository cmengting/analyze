/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_7_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "不得在C语言风格的注释中使用字符序列 /* "
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_2_7_1
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	cppcheckResults, err := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_2_7_1", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Errorf("misra_cpp_2008/rule_2_7_1 Analyze: %v", err)
		return nil, err
	}
	results := generateMisraResult(cppcheckResults)
	return results, nil
}
