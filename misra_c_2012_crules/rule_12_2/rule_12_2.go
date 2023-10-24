/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_12_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.ShiftOp", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule12_2(*csaReports)

	// Change the error message
	for _, result := range results.Results {
		result.ErrorMessage = "[C0604][misra-c2012-12.2]: 移位运算符的右操作数的范围下限为零，上限须比左操作数的基本类型的位宽度小一"
	}

	return results, nil
}
