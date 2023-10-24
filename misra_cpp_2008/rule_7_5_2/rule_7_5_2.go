/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_7_5_2

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	libtooling_results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_7_5_2", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Errorf("failed to execute libtooling: %v", err)
		return nil, err
	}
	return libtooling_results, nil
}
