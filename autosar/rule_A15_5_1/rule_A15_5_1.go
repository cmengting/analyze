/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A15_5_1

import (
	"fmt"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Since clang-tidy would find the main function throwing an exception,
	// which is not what we want. Although this is bad programming practice,
	// it should not be checked by rule_A15_5_1.
	//
	// To solve this, we first find all decls and defs of the main function and
	// consider them as errors, and then we remove these errors from the results
	// of clang-tidy checks (if they do exist).

	AutosarResults, err := runner.RunLibtooling(srcdir, "autosar/rule_A15_5_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}
	args := []string{
		"bugprone-exception-escape",
	}

	ClangTidyResults, err := runner.RunClangTidy(srcdir, args, opts)
	if err != nil {
		return nil, fmt.Errorf("clangtidy err: %v", err)
	}

	// Process ClangTidyResults
	for _, v := range ClangTidyResults.Results {
		v.ErrorMessage = "All user-provided class destructors, deallocation functions, move constructors, move assignment operators and swap functions shall not exit with an exception. A noexcept exception specification shall be added to these functions as appropriate."
	}
	results := &pb.ResultsList{}
	for _, j := range ClangTidyResults.Results {
		found := false
		for _, i := range AutosarResults.Results {
			if j.LineNumber == i.LineNumber && j.Path == i.Path {
				found = true
				break
			}
		}
		if !found {
			results.Results = append(results.Results, j)
		}
	}

	return runner.RemoveDup(results), nil
}
