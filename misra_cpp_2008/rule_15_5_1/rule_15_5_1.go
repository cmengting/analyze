/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_5_1

import (
	"fmt"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	final_res := &pb.ResultsList{}
	results, err := runner.RunClangSema(srcdir, "-fcxx-exceptions -Wexceptions", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	for _, res := range results {
		if strings.Contains(res.Message, "destructor has a implicit non-throwing exception specification") {
			final_res.Results = append(final_res.Results, &pb.Result{
				Path:         res.Filename,
				LineNumber:   int32(res.Line),
				ErrorMessage: "一个类析构函数不应该随着异常退出",
				ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_15_5_1,
			})
		}
	}

	return final_res, nil
}
