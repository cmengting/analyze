/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_7_5_1

import (
	"fmt"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}

	ds, err := runner.RunClangSema(srcdir, "-Wreturn-stack-address", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	for _, d := range ds {
		// To cover four types of warning messages reported by clangsema:
		// 1) warning: address of stack memory associated with local variable 'x' returned
		// 2) warning: reference to stack memory associated with local variable 'x' returned
		// 3) warning: address of stack memory associated with parameter 'x' returned
		// 4) warning: reference to stack memory associated with parameter 'x' returned
		if strings.Contains(d.Message, "stack memory associated with") {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "函数不应返回在该函数中定义的自动变量（包括形参）的引用或指针",
				ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_7_5_1,
			})
		}
	}
	return results, nil
}
