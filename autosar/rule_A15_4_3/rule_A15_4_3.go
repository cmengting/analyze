/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A15_4_3

import (
	"fmt"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	ds, err := runner.RunClangSema(srcdir, "", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	for _, d := range ds {
		if strings.Contains(d.Message, "exception specification in declaration does not match previous declaration") ||
			strings.Contains(d.Message, "is missing exception specification") ||
			strings.Contains(d.Message, "exception specification of overriding function is more lax than base version") {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "The noexcept specification of a function shall either be identical across all translation units, or identical or more restrictive between a virtual member function and an overrider.",
			})
		}
	}
	return runner.SortResult(results), nil
}
