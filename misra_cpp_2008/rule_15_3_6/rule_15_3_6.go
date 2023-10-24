/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_3_6

import (
	"fmt"
	"regexp"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

var pattern = regexp.MustCompile("exception of type .* will be caught by earlier handler")

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	ds, err := runner.RunClangSema(srcdir, "-Wexceptions", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	results := &pb.ResultsList{}
	for _, d := range ds {
		if pattern.MatchString(d.Message) {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "如果在单个 try-catch 语句，或派生类及其部分或全部基类的函数尝试块中，提供多个处理程序，则处理程序应按最派生到基类的顺序排列",
				ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_15_3_6,
			})
		}
	}
	return results, nil
}
