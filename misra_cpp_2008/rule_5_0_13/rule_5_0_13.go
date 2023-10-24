/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_0_13

import (
	"fmt"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	ds, err := runner.RunClangSema(srcdir, "", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	sema_results := &pb.ResultsList{}
	for _, d := range ds {
		if strings.Contains(d.Message, "is not contextually convertible to 'bool'") {
			sema_results.Results = append(sema_results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "if语句的条件和迭代语句的条件必须具有bool类型",
			})
		}
	}

	libtooling_results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_5_0_13", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return sema_results, err
	}

	final_results := &pb.ResultsList{}
	final_results.Results = append(libtooling_results.Results, sema_results.Results...)
	return runner.RemoveDup(final_results), nil
}
