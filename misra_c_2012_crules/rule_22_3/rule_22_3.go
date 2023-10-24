/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_22_3

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.OpenSameFile", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule22_3(*csaReports)

	// Change the error message
	errMsg := "[C0208][misra-c2012-22.3]: 不得在不同文件流上同时打开同一文件进行读写访问"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
