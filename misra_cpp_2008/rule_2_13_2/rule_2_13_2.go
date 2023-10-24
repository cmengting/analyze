/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_13_2

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra_c_2012_crules/rule_7_1"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_2_13_2", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return results, fmt.Errorf("run libtooling err: %v", err)
	}
	rs, err := rule_7_1.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range rs.Results {
		results.Results = append(results.Results, &pb.Result{
			Path:         r.Path,
			LineNumber:   r.LineNumber,
			ErrorMessage: "不得使用八进制常量（除零以外）和八进制转义序列（除“\\0”以外）",
			ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_2_13_2,
		})
	}
	return results, nil
}
