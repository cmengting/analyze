/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package g1153

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reports, err := runner.RunCpplint(srcdir, "-,+build/header_guard", opts)
	if err != nil {
		return nil, err
	}
	resultsSet := pb.NewResultsSet()
	for _, r := range reports {
		resultsSet.Add(&pb.Result{
			Path:         r.Path,
			LineNumber:   int32(r.LineNumber),
			ErrorMessage: "All header files should have #define guards to prevent multiple inclusion. The format of the symbol name should be <PROJECT>_<PATH>_<FILE>_H_.",
		})
	}
	return &resultsSet.ResultsList, nil
}
