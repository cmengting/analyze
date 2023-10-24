/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A14_7_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_cpp_2008_rule_14_7_3 "naive.systems/analyzer/misra_cpp_2008/rule_14_7_3"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_cpp_2008_rule_14_7_3.Analyze(srcdir, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "Template specialization shall be declared in the same file (1) as the primary template (2) as a user-defined type, for which the specialization is declared."
	}
	return results, nil
}
