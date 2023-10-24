/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_0_5

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_21_4"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	rs, err := rule_21_4.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range rs.Results {
		r.ErrorMessage = "不得使用 setjmp 宏和 longjmp 函数"
		r.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_17_0_5
	}
	return rs, nil
}
