/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_6_4_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra_c_2012_crules/rule_16_1"
)

func append_results(results *pb.ResultsList, other_results *pb.ResultsList) {
	results.Results = append(results.Results, other_results.Results...)
}

func run_cppcheck_misra_rules(name, srcdir string, results *pb.ResultsList, opts *options.CheckOptions) error {
	other_results, err := runner.RunCppcheck(srcdir, name, checker_integration.Cppcheck_STU, opts)
	if err != nil {
		return err
	}
	append_results(results, other_results)
	return nil
}

func run_libtooling_misra_rules_6_4_6(srcdir string, results *pb.ResultsList, opts *options.CheckOptions) error {
	other_results, err := runner.RunLibtooling(srcdir, "misra_cpp_2008/rule_6_4_6", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return err
	}

	other_results_fine := &pb.ResultsList{}

	for _, result := range other_results.Results {
		if result.ExternalMessage == "" {
			other_results_fine.Results = append(other_results_fine.Results, result)
		}
	}

	append_results(results, other_results_fine)
	return nil
}

func run_clang_sema_with_error_message(name, srcdir string, results *pb.ResultsList, opts *options.CheckOptions) error {
	ds, err := runner.RunClangSema(srcdir, "", opts)
	if err != nil {
		return err
	}
	for _, d := range ds {
		if d.Message == name {
			results.Results = append(results.Results, &pb.Result{
				Path:         d.Filename,
				LineNumber:   int32(d.Line),
				ErrorMessage: "switch语句必须为良构（well-formed）",
				ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_6_4_3,
			})
		}
	}
	return nil
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := rule_16_1.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	// check for missing break or throw statement
	err = run_cppcheck_misra_rules("misra_cpp_2008/rule_6_4_5", srcdir, results, opts)
	if err != nil {
		return nil, err
	}
	// check for missing default statement
	err = run_libtooling_misra_rules_6_4_6(srcdir, results, opts)
	if err != nil {
		return nil, err
	}
	// check for no case label in switch statement
	err = run_cppcheck_misra_rules("misra_cpp_2008/rule_6_4_8", srcdir, results, opts)
	if err != nil {
		return nil, err
	}

	// check for jump from switch statement to another case label and jump pass init
	err = run_clang_sema_with_error_message("cannot jump from switch statement to this case label", srcdir, results, opts)
	if err != nil {
		return nil, err
	}
	for _, result := range results.Results {
		result.ErrorMessage = "switch语句必须为良构（well-formed）"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_6_4_3
	}
	return runner.SortResult(runner.RemoveDup(results)), nil
}
