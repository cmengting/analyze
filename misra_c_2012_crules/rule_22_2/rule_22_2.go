/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_22_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=unix.Malloc", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule22_2(*csaReports)

	// Change the error message
	errMsg := "[C0209][misra-c2012-22.2]: 只有通过标准库函数分配的内存块才能被释放"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
