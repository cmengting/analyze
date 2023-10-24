/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A2_5_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := runner.RunMisra(srcdir, "misra_cpp_2008/rule_2_5_1", opts)
	for _, result := range results.Results {
		result.ErrorMessage = "Digraphs shall not be used."
	}
	return results, nil
}
