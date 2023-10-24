/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A7_3_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_7_3_5", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "All overloads of a function shall be visible from where it is called."
	}
	results1, err := runner.RunLibtooling(srcdir, "autosar/rule_A7_3_1", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		return nil, err
	}
	results.Results = runner.MergeResults(results.Results, results1.Results)
	return results, nil
}
