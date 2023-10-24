/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_16_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_2"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_3"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_4"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_5"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_6"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	rule_16_2_results, _ := rule_16_2.Analyze(srcdir, opts)
	rule_16_3_results, _ := rule_16_3.Analyze(srcdir, opts)
	rule_16_4_results, _ := rule_16_4.Analyze(srcdir, opts)
	rule_16_5_results, _ := rule_16_5.Analyze(srcdir, opts)
	rule_16_6_results, _ := rule_16_6.Analyze(srcdir, opts)
	rule_16_1_results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_16_1", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := &pb.ResultsList{}
	results.Results = append(results.Results, rule_16_1_results.Results...)
	results.Results = append(results.Results, rule_16_2_results.Results...)
	results.Results = append(results.Results, rule_16_3_results.Results...)
	results.Results = append(results.Results, rule_16_4_results.Results...)
	results.Results = append(results.Results, rule_16_5_results.Results...)
	results.Results = append(results.Results, rule_16_6_results.Results...)
	for _, result := range results.Results {
		result.ErrorMessage = "[C1907][misra-c2012-16.1]: 所有switch语句必须为良构"
		result.ErrorKind = pb.Result_MISRA_C_2012_RULE_16_1
	}
	return runner.SortResult(runner.RemoveDup(results)), nil
}
