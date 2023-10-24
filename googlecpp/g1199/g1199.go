/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package g1199

import (
	"fmt"
	"regexp"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	ds, err := runner.RunClangSema(srcdir, "-Wint-conversion -Wimplicit-int-conversion -Wshorten-64-to-32 -Wimplicit-int-float-conversion -Wsign-conversion -Wconstant-conversion -Wfloat-conversion -Wimplicit-float-conversion", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	reg := regexp.MustCompile(`'[a-zA-Z0-9\s\_]+' to '[a-zA-Z0-9\s\_]+'|integer`)
	for _, d := range ds {
		match := reg.FindString(d.Message)
		if strings.Contains(match, "short") || strings.Contains(match, "int") || strings.Contains(match, "long") || strings.Contains(match, "char") {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "Use care when converting integer types",
			})
		}
	}
	return runner.SortResult(runner.RemoveDup(results)), nil
}
