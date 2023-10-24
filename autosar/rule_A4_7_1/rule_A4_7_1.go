/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A4_7_1

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	libtoolingResults, err := runner.RunLibtooling(srcdir, "autosar/rule_A4_7_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	results := runner.RemoveDup(libtoolingResults)
	ds, err := runner.RunClangSema(srcdir, "-Wconversion", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	for _, d := range ds {
		results.Results = append(results.Results, &pb.Result{
			Path:         d.Filename,
			LineNumber:   int32(d.Line),
			ErrorMessage: "An integer expression shall not lead to data loss.",
		})
	}
	results = runner.SortResult(results)
	results = runner.RemoveDup(results)
	return results, nil
}
