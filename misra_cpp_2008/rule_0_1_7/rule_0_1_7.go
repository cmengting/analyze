/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_0_1_7

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_17_7"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := rule_17_7.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "具有非void返回类型的函数，只要不是重载运算符，那么它返回的值必须被使用"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_0_1_7
	}
	// misra C implement will try to match the use of function, like 'f(a, b)'. So
	// the operator overload in C++ will not be match. We can reuse it for misra C++.
	return results, nil
}
