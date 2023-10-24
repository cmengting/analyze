/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_18_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.PointerArithMisra", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule18_1(*csaReports)

	// Change the error messages
	errMsg := "[C1308][misra-c2012-18.1]: 对指针操作数进行算术运算得来的指针只得用于寻址与该指针操作数同一数组的元素"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
