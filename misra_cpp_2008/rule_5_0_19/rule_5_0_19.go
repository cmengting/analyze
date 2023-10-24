/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_0_19

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	misra_c_2012_crules_rule_18_5 "naive.systems/analyzer/misra_c_2012_crules/rule_18_5"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := misra_c_2012_crules_rule_18_5.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "对象声明包含的间接指针不得超过两级"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_0_19
	}
	return results, err
}
