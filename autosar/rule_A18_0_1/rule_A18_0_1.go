/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A18_0_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_18_0_1 "naive.systems/analyzer/misra_cpp_2008/rule_18_0_1"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_18_0_1.Analyze(srcdir, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, r := range results.Results {
		r.ErrorMessage = "The C library facilities shall only be accessed through C++ library headers."
	}
	return results, nil
}
