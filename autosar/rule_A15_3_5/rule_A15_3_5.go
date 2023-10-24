/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A15_3_5

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_15_3_5 "naive.systems/analyzer/misra_cpp_2008/rule_15_3_5"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_15_3_5.Analyze(srcdir, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "A class type exception shall be caught by reference or const reference."
	}
	return results, nil
}
