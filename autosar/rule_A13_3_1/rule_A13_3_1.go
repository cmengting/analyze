/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A13_3_1

import (
	"fmt"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"strings"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {

	AutosarResults, err := runner.RunLibtooling(srcdir, "autosar/rule_A13_3_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}

	args := []string{
		"bugprone-forwarding-reference-overload",
	}
	ClangTidyResults, err := runner.RunClangTidy(srcdir, args, opts)
	if err != nil {
		return nil, fmt.Errorf("clangtidy err: %v", err)
	}
	for _, v := range ClangTidyResults.Results {
		v.ErrorMessage = "[autosar-A13-3-1]" + v.ErrorMessage
	}

	results := &pb.ResultsList{}
	results.Results = append(results.Results, AutosarResults.Results...)
	results.Results = append(results.Results, ClangTidyResults.Results...)

	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[autosar-A13-3-1]") {
			v.ErrorMessage = "A function that contains \"forwarding reference\" as its argument shall not be overloaded."
		}
	}

	return runner.RemoveDup(results), err
}
