/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_4

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	diagnostics, err := runner.RunClangSema(srcdir, "-Wreturn-type", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	results := clangsema.CheckRule17_4(diagnostics)

	// Change the error message
	errMsg := "violation of misra-c2012-17.4"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
