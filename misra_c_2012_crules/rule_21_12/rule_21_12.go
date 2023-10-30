/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_21_12

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	if opts.JsonOption.Standard == "c89" || opts.JsonOption.Standard == "c90" {
		return &pb.ResultsList{}, nil
	}

	results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_21_12", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C0409][misra-c2012-21.12]") {
			v.ErrorMessage = "[C0409][misra-c2012-21.12]: 不应使用 <fenv.h> 的异常处理特性"
		}
	}
	return results, nil
}
