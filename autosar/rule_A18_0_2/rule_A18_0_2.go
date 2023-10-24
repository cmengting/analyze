/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A18_0_2

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// atof, atoi and atol shall not be used
	args := []string{
		"cert-err34-c",
	}
	results, err := runner.RunClangTidy(srcdir, args, opts)
	if err != nil {
		return nil, fmt.Errorf("clangtidy err: %v", err)
	}
	for _, results := range results.Results {
		results.ErrorMessage = "The error state of a conversion from string to a numeric value shall be checked."
	}

	// stream related conversion
	libtoolingResults, err := runner.RunLibtooling(srcdir, "autosar/rule_A18_0_2", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	results.Results = append(results.Results, libtoolingResults.Results...)

	return results, nil
}
