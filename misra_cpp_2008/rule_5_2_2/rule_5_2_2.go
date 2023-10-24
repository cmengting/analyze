/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_2_2

import (
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func formatReportMessage(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "一个指向虚基类（virtual base class）的指针只能通过dynamic_cast的方式被转换为一个指向派生类（derived class）的指针"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_2_2
	}

	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	result, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_5_2_2", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	resultFromClang, err := runner.RunClangForErrorsOrWarnings(srcdir, true, "", opts)
	if err != nil {
		return nil, err
	}
	for _, clangErr := range resultFromClang.Results {
		if strings.Contains(clangErr.ErrorMessage, "via virtual base") {
			result.Results = append(result.Results, clangErr)
		}
	}

	return formatReportMessage(runner.SortResult(result)), nil
}
