/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_13_1

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

	results, err := runner.RunLibtooling(srcdir, "misra/rule_13_1", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1606][misra-c2012-13.1]") {
			v.ErrorMessage = "[C1606][misra-c2012-13.1]: 初始化器列表不得含有持续的副作用"
		}
	}
	return results, nil
}
