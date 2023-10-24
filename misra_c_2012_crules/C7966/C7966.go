/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package C7966

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func alterErrorMsg(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "malloc分配的大小应为4的倍数"
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results_libtooling, err := runner.RunLibtooling(srcdir, "misra/C7966", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}

	results_cppcheck, err := runner.RunCppcheck(srcdir, "naivesystems/C7966", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Errorf("error when executing cppcheck: %v", err)
		return nil, err
	}
	results_libtooling.Results = append(results_libtooling.Results, results_cppcheck.Results...)

	final := &pb.ResultsList{}
	allKeys := make(map[string]bool)
	for _, res := range results_libtooling.Results {
		key := (res.Filename + string(res.LineNumber))
		if _, value := allKeys[key]; !value {
			allKeys[key] = true
			final.Results = append(final.Results, res)
		}
	}
	return alterErrorMsg(final), nil
}
