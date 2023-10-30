/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_1_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra_c_2012_crules/rule_5_1"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err1 := runner.RunLibtooling(srcdir, "misra/rule_1_1", checker_integration.Libtooling_STU, opts)
	if err1 != nil {
		glog.Error(err1)
		return nil, err1
	}
	rule_5_1_results, err2 := rule_5_1.Analyze(srcdir, opts)
	if err2 != nil {
		glog.Error(err2)
		return nil, err2
	}
	for _, result := range rule_5_1_results.Results {
		result.ErrorMessage = "[C2201][misra-c2012-1.1]: The program shall contain no violations of the standard C syntax and constraints, and shall not exceed the implementation's translation limits"
		result.ErrorKind = pb.Result_MISRA_C_2012_RULE_1_1_EXTERN_ID_CHAR
	}
	results.Results = append(results.Results, rule_5_1_results.Results...)
	return results, nil
}
