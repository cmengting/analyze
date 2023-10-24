/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_14_3

import (
	"fmt"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/analyzer"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clangsema
	diagnostics, err := runner.RunClangSema(srcdir, "-Wtautological-constant-out-of-range-compare -Wtautological-unsigned-zero-compare -Wtautological-type-limit-compare -Wtautological-overlap-compare -Wtautological-constant-compare", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	clangsemaResults := clangsema.CheckRule14_3(diagnostics)

	// libtooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_14_3", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// merge differenct checker results
	resultsList := &pb.ResultsList{}
	resultsList.Results = append(resultsList.Results, clangsemaResults.Results...)
	resultsList.Results = append(resultsList.Results, libtoolingResults.Results...)

	// Infer
	if opts.JsonOption.AggressiveMode != nil && *opts.JsonOption.AggressiveMode {
		inferReports, err := runner.RunInfer(srcdir, "--bufferoverrun-only --debug-exceptions", opts)
		if err != nil {
			glog.Error(err)
			return nil, err
		}
		inferResults := infer.CheckRule14_3(*inferReports)
		resultsList.Results = append(resultsList.Results, inferResults.Results...)
	}

	// change the error message
	errMsg := "[C1702][misra-c2012-14.3]: 控制表达式不得为不变量"
	for _, result := range resultsList.Results {
		result.ErrorMessage = errMsg
	}

	// delete false-positives
	err = analyzer.DeleteFalsePositiveResults(resultsList)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// remove duplicate
	resultsList = runner.RemoveDup(resultsList)

	return runner.SortResult(resultsList), nil
}
