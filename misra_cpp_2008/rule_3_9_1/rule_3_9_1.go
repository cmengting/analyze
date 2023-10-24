/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_3_9_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Refer to the checker of MISRA_C_2012 rule 8.3: All declarations of an object or function
	// shall use the same names and type qualifiers.
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_3_9_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range results.Results {
		r.ErrorMessage = "在所有声明和重新声明中，用于对象、函数返回类型或函数参数的类型的每个词符必须对应相同"
		r.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_3_9_1
	}
	return results, nil
}
