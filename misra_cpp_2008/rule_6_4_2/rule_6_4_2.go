/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_6_4_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_15_7"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "所有if … else if结构都必须以一个else语句终止"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_6_4_2
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	misraResults, err := rule_15_7.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	results := generateMisraResult(misraResults)
	return results, nil
}
