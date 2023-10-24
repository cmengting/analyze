/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A7_2_1

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	result_csa, err := runner.RunCSA(srcdir, "-analyzer-checker=alpha.cplusplus.EnumCastOutOfRange", opts)
	if err != nil {
		return nil, err
	}
	final_csa := runner.KeepNeededErrorByFilterTargetMsgInCSAReports(result_csa, "The value provided to the cast expression is not in the valid range of values for the enum", "An expression with enum underlying type shall only have values.")
	return final_csa, nil
}
