/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_17_2

import (
	"fmt"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	MisraResults := runner.RunMisra(srcdir, "misra_c_2012/rule_17_2", opts)

	args := []string{
		"misc-no-recursion",
	}
	ClangTidyResults, err := runner.RunClangTidy(srcdir, args, opts)
	if err != nil {
		return nil, fmt.Errorf("clangtidy err: %v", err)
	}
	for _, v := range ClangTidyResults.Results {
		v.ErrorMessage = "[C1507][misra-c2012-17.2]" + v.ErrorMessage
	}

	results := &pb.ResultsList{}
	results.Results = append(results.Results, MisraResults.Results...)
	results.Results = append(results.Results, ClangTidyResults.Results...)

	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1507][misra-c2012-17.2]") {
			v.ErrorMessage = "[C1507][misra-c2012-17.2]: 函数不得直接或间接调用自身"
		}
	}

	results = runner.RemoveDup(results)

	return results, nil
}
