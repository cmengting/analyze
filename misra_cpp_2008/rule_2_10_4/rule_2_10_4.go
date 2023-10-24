/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_2_10_4

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_5_7"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "类，联合以及枚举类型的名称应当是唯一的"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_2_10_4
	}

	return results
}

func Analyze(srcdir string, options *options.CheckOptions) (*pb.ResultsList, error) {
	misraResults, err := rule_5_7.Analyze(srcdir, options)
	if err != nil {
		return nil, err
	}

	results := generateMisraResult(misraResults)
	return results, nil
}
