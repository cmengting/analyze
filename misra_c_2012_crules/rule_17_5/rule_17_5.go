/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_5

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/gcc_diagnostics"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// GCC
	diagnostics, err := runner.RunGCC(opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	gccResults := gcc_diagnostics.CheckRule17_5(diagnostics)

	// LibTooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_17_5", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// Merge the different checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, gccResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// Change the error message
	for _, result := range results.Results {
		result.ErrorMessage = "[C1504][misra-c2012-17.5]: 如果函数形参被声明为数组类型，那么对应的实参必须具有适当数量的元素"
	}

	results = runner.RemoveDup(results)
	return results, nil
}
