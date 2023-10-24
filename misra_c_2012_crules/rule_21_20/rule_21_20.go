/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_20

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.DeprecatedResource", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule21_20(*csaReports)

	// Change the error messages
	errMsg := "[C0401][misra-c2012-21.20]: 标准库函数 asctime，ctime，gmtime，localtime，localeconv，getenv，setlocale 或 strerror 返回的指针后面不得紧跟对同一函数的调用"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
