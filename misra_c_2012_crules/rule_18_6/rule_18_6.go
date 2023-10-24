/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_18_6

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=core.StackAddressEscape", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule18_6(*csaReports)

	// Change the error messages
	errMsg := "[C1303][misra-c2012-18.6]: 不得将自动存储对象的地址复制给在该对象不复存在后仍然存在的另一个对象"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
