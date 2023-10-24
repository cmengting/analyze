/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_3_2_4

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_8_6"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "一个具有外部链接的标识符必须有且仅有一个定义"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_3_2_4
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	misraResults, err := rule_8_6.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	results := generateMisraResult(misraResults)
	return results, nil
}
