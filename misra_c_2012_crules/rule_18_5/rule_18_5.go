/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_18_5

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra/rule_18_5", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	for _, v := range results.Results {
		v.ErrorMessage = "[C1304][misra-c2012-18.5]: 声明应含有最多两层嵌套指针"
	}
	return results, nil
}
