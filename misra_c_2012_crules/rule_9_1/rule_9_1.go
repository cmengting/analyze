/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_9_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=core.UndefinedBinaryOperatorResult -analyzer-checker=core.uninitialized", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule9_1(*csaReports)

	// Change the error message
	errMsg := "[C1205][misra-c2012-9.1]: 对于具有自动存储周期的对象，不得在设定它的值之前读取它的值"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
