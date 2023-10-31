/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_8_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/gcc_diagnostics"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// GCC
	gccDiagnostics, err := runner.RunGCC(opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	gccResults := gcc_diagnostics.CheckRule8_1(gccDiagnostics)

	// Misra
	misraResults := runner.RunMisra(srcdir, "misra_c_2012/rule_8_1", opts)

	// Merge different checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, gccResults.Results...)
	results.Results = append(results.Results, misraResults.Results...)

	// Change the error message
	errMsg := "[C0514][misra-c2012-8.1]: 必须明确指定类型"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	// Remove duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
