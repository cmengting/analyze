/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_18_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.PointerSubMisra", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule18_2(*csaReports)

	// Change the error messages
	errMsg := "[C1307][misra-c2012-18.2]: 只有寻址同一数组元素的指针之间才能进行减法运算"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
