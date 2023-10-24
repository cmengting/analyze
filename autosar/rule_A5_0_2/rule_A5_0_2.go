/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A5_0_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_5_0_13 "naive.systems/analyzer/misra_cpp_2008/rule_5_0_13"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_5_0_13.Analyze(srcdir, opts)
	for _, result := range results.Results {
		result.ErrorMessage = "The condition of an if-statement and the condition of an iteration statement shall have type bool."
	}
	return results, err
}
