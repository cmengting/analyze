/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_18

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/csa"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// CSA
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=core,alpha.unix.cstring.OutOfBounds", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule21_18(*csaReports)

	// Infer
	inferReports, err := runner.RunInfer(srcdir, "--bufferoverrun --debug-exceptions", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	inferResults := infer.CheckRule21_18(*inferReports)

	// libtooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_21_18", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// Merge different checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, csaResults.Results...)
	results.Results = append(results.Results, inferResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// Change the error message
	errMsg := "[C0403][misra-c2012-21.18]: 如果要将 size_t 实参传递给 <cstring.h> 中的任意函数，则 size_t 实参必须具有恰当的值"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	// Remove duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
