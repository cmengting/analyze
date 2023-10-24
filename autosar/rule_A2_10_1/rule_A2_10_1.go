/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A2_10_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_2_10_2", checker_integration.Libtooling_CTU, opts)
	for _, result := range results.Results {
		result.ErrorMessage = "An identifier declared in an inner scope shall not hide an identifier declared in an outer scope."
	}
	if err != nil {
		return nil, err
	}
	return results, nil
}
