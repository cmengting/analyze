/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_0_1_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_cxx_2008.UnreachableBranch", opts)
	if err != nil {
		return nil, err
	}
	results := runner.KeepNeededErrorByFilterTargetMsgInCSAReports(reports, ".*", "shall not contain infeasible paths")
	for _, result := range results.Results {
		result.ErrorMessage = "项目不得含有不可行路径"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_0_1_2
	}
	return results, nil
}
