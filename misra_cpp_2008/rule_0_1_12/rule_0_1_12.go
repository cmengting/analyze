/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_0_1_12

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, options *options.CheckOptions) (*pb.ResultsList, error) {
	resultsOfLibtooling, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_0_1_12", checker_integration.Libtooling_CTU, options)
	if err != nil {
		glog.Errorf("failed to execute libtooling: %v", err)
		return nil, err
	}
	return resultsOfLibtooling, nil
}
