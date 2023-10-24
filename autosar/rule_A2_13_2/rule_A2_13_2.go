/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A2_13_2

import (
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := pb.ResultsList{}
	resultFromClang, err := runner.RunClangForErrorsOrWarnings(srcdir, true, "", opts)
	if err != nil {
		return nil, err
	}
	for _, clangErr := range resultFromClang.Results {
		if strings.Contains(clangErr.ErrorMessage, "unsupported non-standard concatenation of string literals") {
			clangErr.ErrorMessage = "String literals with different encoding prefixes shall not be concatenated."
			results.Results = append(results.Results, clangErr)
		}
	}
	return runner.RemoveDup(&results), err
}
