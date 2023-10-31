/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_3

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clangsema
	diagnostics, err := runner.RunClangSema(srcdir, "-Wimplicit-function-declaration", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	results := clangsema.CheckRule17_3(diagnostics)

	// Change the error message
	errMsg := "[C1506][misra-c2012-17.3]: 不得隐式声明函数"
	for _, result := range results.Results {
		result.ErrorMessage = errMsg
	}
	return results, nil
}
