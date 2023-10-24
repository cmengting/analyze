/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_18_3

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.PointerComp", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	results := csa.CheckRule18_3(*csaReports)

	// Change the error messages
	errMsg := "[C1306][misra-c2012-18.3]: 大小比较运算符 >，>=，< 和 <= 不得用于两个指针类型的对象，除非这两个指针指向同一对象"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
