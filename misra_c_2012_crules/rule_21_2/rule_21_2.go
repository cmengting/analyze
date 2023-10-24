/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_2

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_21_2", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	errMsg := "[C0419][misra-c2012-21.2]: 不得声明保留标识符（reserved identifier）和保留宏名称（reserved macro name）"
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C0419][misra-c2012-21.2]") {
			v.ErrorMessage = errMsg
		}
	}
	return results, nil
}
