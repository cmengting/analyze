/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_2

import (
	"fmt"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Infer
	inferReports, err := runner.RunInfer(srcdir, "--liveness", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	inferResults := infer.CheckRule2_2(*inferReports)

	// Clangsema
	diagnostics, err := runner.RunClangSema(srcdir, "-Wunused-value", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	clangsemaResults := clangsema.CheckRule2_2(diagnostics)

	// LibTooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_2_2", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// merge differenct checker results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, inferResults.Results...)
	results.Results = append(results.Results, clangsemaResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// change the error message
	errMsg := "[C2006][misra-c2012-2.2]: 不得有死代码"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	// remove duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
