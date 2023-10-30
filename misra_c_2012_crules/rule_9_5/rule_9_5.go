/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_9_5

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

	results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_9_5", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1201][misra-c2012-9.5]") {
			v.ErrorMessage = "[C1201][misra-c2012-9.5]: 对数组对象进行指定初始化时，必须显式指定数组大小"
		}
	}
	return results, nil
}
