/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package g1175

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reports, err := runner.RunCpplint(srcdir, "-,+readability/inheritance", opts)
	if err != nil {
		return nil, err
	}
	type Loc struct {
		string
		int32
	}
	results := make(map[Loc]struct{})
	resultsList := &pb.ResultsList{}
	report := func(path string, line int32) {
		loc := Loc{path, line}
		if _, reported := results[loc]; !reported {
			results[loc] = struct{}{}
			resultsList.Results = append(resultsList.Results, &pb.Result{
				Path:         path,
				LineNumber:   line,
				ErrorMessage: "Do not use virtual when declaring an override",
			})
		}
	}
	for _, r := range reports {
		report(r.Path, int32(r.LineNumber))
	}
	return resultsList, nil
}
