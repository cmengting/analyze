/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_19_1

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=alpha.unix.cstring.BufferOverlap", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule19_1(*csaReports)

	// LibTooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_19_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// Merge different checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, csaResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// Change the error messages
	errMsg := "[C0302][misra-c2012-19.1]: 不得将对象赋值或复制给与其重叠的对象"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}

	// Remove duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
