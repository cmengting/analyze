/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_13

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.CtypeFunctionArgs", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule21_13(*csaReports)

	// Change the error messages
	errMsg := "[C0408][misra-c2012-21.13]: 传递给 <ctype.h> 函数的值必须能够表示为无符号 char 或 EOF 类型"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
