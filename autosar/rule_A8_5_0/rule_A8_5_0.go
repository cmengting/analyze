/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A8_5_0

import (
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	checkers := []string{
		"core.UndefinedBinaryOperatorResult",
		"core.CallAndMessage",
		"core.NullDereference",
		"core.uninitialized",
		"cplusplus.InnerPointer",
		"cwe.MissInit",
	} // refers to cwe-456 and misra-c2012-9.1

	result_csa, err := runner.RunCSA(srcdir, "-analyzer-checker="+strings.Join(checkers[:], ","), opts)
	if err != nil {
		return nil, err
	}
	final_csa := runner.KeepNeededErrorByFilterTargetMsgInCSAReports(result_csa, "[undefined|uninitialized|garbage]", "All memory shall be initialized before it is read.")
	return final_csa, nil
}
