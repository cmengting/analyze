/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_13_4

import (
	"fmt"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clangsema
	clangsemaDiagnostics, err := runner.RunClangSema(srcdir, "-Wparentheses", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	clangsemaResults := clangsema.CheckRule13_4(clangsemaDiagnostics)

	// LibTooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_13_4", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// Merge differenct checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, clangsemaResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// Change the error message
	for _, result := range results.Results {
		result.ErrorMessage = "[C1603][misra-c2012-13.4]: 不得使用赋值运算符的结果"
	}
	return results, nil
}
