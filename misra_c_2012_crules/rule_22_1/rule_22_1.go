/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_22_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// CSA
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=unix.Malloc", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule22_1(*csaReports)

	// Infer
	inferReports, err := runner.RunInfer(srcdir, "--pulse", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	inferResults := infer.CheckRule22_1(*inferReports)

	// Merge different checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, csaResults.Results...)
	results.Results = append(results.Results, inferResults.Results...)

	// Change the error message
	errMsg := "[C0210][misra-c2012-22.1]: 通过标准库函数动态获取的所有资源都必须被显式释放"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	return results, nil
}
