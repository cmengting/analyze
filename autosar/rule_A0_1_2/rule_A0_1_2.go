/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A0_1_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_0_1_7 "naive.systems/analyzer/misra_cpp_2008/rule_0_1_7"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_0_1_7.Analyze(srcdir, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "The value returned by a function having a non-void return type that is not an overloaded operator shall be used"
	}
	return results, nil
}
