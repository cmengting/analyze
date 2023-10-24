/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_5

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_2_5", checker_integration.Cppcheck_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	filteredResults := []*pb.Result{}
	for _, result := range results.Results {
		// The flag is not specified, with the false default value.
		if strings.HasSuffix(result.ErrorMessage, "(optional)") &&
			(opts.JsonOption.CheckIncludeGuards == nil ||
				!*opts.JsonOption.CheckIncludeGuards) {
			continue
		}
		// The error is not in include guards, reported anyway.
		// Or the flag is specified as true,
		// and results with an 'optional' mark should be reported.
		filteredResults = append(filteredResults, result)
	}
	results.Results = filteredResults
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C2003][misra-c2012-2.5]") {
			v.ErrorMessage = "[C2003][misra-c2012-2.5]: 项目不应含有未使用的宏声明"
		}
	}
	return results, nil
}
