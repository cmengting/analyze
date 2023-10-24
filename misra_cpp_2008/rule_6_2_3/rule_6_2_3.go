/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_6_2_3

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "在预处理之前，null语句只能单独出现在一行中；如果null语句之后的第一个字符是一个空格字符，后面可以有一条注释"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_6_2_3
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	cppcheck_res, err := runner.RunCppcheck(srcdir, "misra_cpp_2008/rule_6_2_3", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Errorf("misra_cpp_2008/rule_6_2_3 Analyze: %v", err)
		return nil, err
	}
	results := generateMisraResult(cppcheck_res)
	return results, nil
}
