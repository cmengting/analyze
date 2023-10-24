/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// CSA
	csaReport, err := runner.RunCSA(srcdir, "-analyzer-checker=alpha.deadcode.UnreachableCode", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule2_1(*csaReport)

	// Infer
	inferReports, err := runner.RunInfer(srcdir, "--bufferoverrun-only --debug-exceptions", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	inferResults := infer.CheckRule2_1(*inferReports)

	// Merge
	results := &pb.ResultsList{}
	results.Results = append(results.Results, csaResults.Results...)
	results.Results = append(results.Results, inferResults.Results...)
	errMsg := "[C2007][misra-c2012-2.1]: 项目不得含有不可达代码"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	// Remove Duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
