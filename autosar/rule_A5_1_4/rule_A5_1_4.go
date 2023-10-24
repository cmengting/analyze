/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A5_1_4

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// This rule implements mainly in three steps.
	// Firstly, get all referenced captures from the code by libtooling.
	results, err := runner.RunLibtooling(srcdir, "autosar/rule_A5_1_4", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		return nil, err
	}
	// Secondly, use CSA to find all potential stack address escape errors.
	results_csa, err := runner.RunCSA(srcdir, "-analyzer-checker=core.StackAddressEscape", opts)
	if err != nil {
		return nil, err
	}
	final_csa := runner.KeepNeededErrorByFilterTargetMsgInCSAReports(results_csa, "Address of stack memory associated with local variable '\\S+' is still referred to by the stack variable '\\S+' upon returning to the caller.  This will be a dangling reference", "A lambda expression object shall not outlive any of its reference-captured objects.")
	// Finally, compare the above two results, so the checker could filter the
	// libtooling results based on the CSA results, to find the real location
	// where exists the risks of memory leakage.
	final_results_list := []*pb.Result{}
	for _, result := range results.Results {
		for _, csa_result := range final_csa.Results {
			if result.GetFilename() == csa_result.GetFilename() && result.GetLineNumber() == csa_result.GetLineNumber() {
				final_results_list = append(final_results_list, result)
				break
			}
		}
	}
	results.Results = final_results_list
	return results, nil
}
