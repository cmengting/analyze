/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_3_1

import (
	"fmt"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	ds, err := runner.RunGCC(opts)
	if err != nil {
		return results, fmt.Errorf("gcc executing err: %v", err)
	}
	for _, d := range ds {
		if d.Option == "-Wterminate" && d.Message == "‘throw’ will always call ‘terminate’" {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Locations[0].Caret.File,
				LineNumber:   int32(d.Locations[0].Caret.Line),
				ErrorMessage: "只有在程序启动后和程序终止前才应引发异常",
				ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_15_3_1,
			})
		}
	}
	return results, nil
}
