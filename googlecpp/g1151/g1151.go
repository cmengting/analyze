/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package g1151

import (
	"os"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	final_results := &pb.ResultsList{}

	// Generate a random file name, and store it in the `JsonOption`. If it needs
	// to assign the file with a specific name, just replace the `OptionalInfoFile`
	// with the filename string you want.
	f, err := os.CreateTemp(opts.EnvOption.ResultsDir, "tmp-*.txt")
	if err != nil {
		return nil, err
	}
	opts.JsonOption.OptionalInfoFile = f.Name()
	f.Close()

	results_lib, err := runner.RunLibtooling(srcdir, "googlecpp/g1151", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}

	results_header_compiler, err := runner.RunHeaderCompile(srcdir, "googlecpp/g1151", opts)
	if err != nil {
		return nil, err
	}
	for _, r := range results_header_compiler.Results {
		r.ErrorMessage = "Header files should be self-contained (compile on their own) and end in .h"
		if r.ExternalMessage != "" {
			r.ErrorMessage += "\n" + r.ExternalMessage
		}
	}

	final_results.Results = runner.MergeResults(results_lib.Results, results_header_compiler.Results)
	return final_results, nil
}
