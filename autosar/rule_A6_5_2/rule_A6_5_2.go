/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A6_5_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_6_5_1 "naive.systems/analyzer/misra_cpp_2008/rule_6_5_1"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_6_5_1.Analyze(srcdir, opts)
	for _, result := range results.Results {
		result.ErrorMessage = "A for loop shall contain a single loop-counter which shall not have floating-point type."
	}
	return results, err
}
