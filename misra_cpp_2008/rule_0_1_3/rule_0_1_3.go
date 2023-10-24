/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_0_1_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	// It seems that cppcheck cannot specify enable only one rule id.
	cppcheck_results, err := runner.RunCppcheck(srcdir, "--enable=style", checker_integration.Cppcheck_Binary, opts)
	if err != nil {
		return nil, err
	}
	for _, r := range cppcheck_results.Results {
		if r.Name != "unusedStructMember" {
			continue
		}
		r.ErrorMessage = "shall not contain unused variables"
		results.Results = append(results.Results, r)
	}
	ds, err := runner.RunClangSema(srcdir, "-Wunused -Wunused-but-set-parameter -Wunused-parameter", opts)
	if err != nil {
		return nil, err
	}
	for _, d := range ds {
		results.Results = append(results.Results, &pb.Result{
			Path:         d.Filename,
			LineNumber:   int32(d.Line),
			ErrorMessage: "shall not contain unused variables",
		})
	}
	for _, result := range results.Results {
		result.ErrorMessage = "项目不得含有未使用的变量"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_0_1_3
	}
	return results, nil
}
