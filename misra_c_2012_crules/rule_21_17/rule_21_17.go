/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_17

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.CStringBoundMisra", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule21_17(*csaReports)

	// Change the error messages
	errMsg := "[C0404][misra-c2012-21.17]: 使用 <cstring.h> 的字符串处理函数不得产生超出指针形参引用对象边界的访问"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
