/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_8_4_3

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/rule_17_4"
)

func generateMisraResult(results *pb.ResultsList) *pb.ResultsList {
	for _, result := range results.Results {
		result.ErrorMessage = "所有函数退出路径，如果为非空（non-void）返回类型，则必须有一个包含表达式的显式return语句"
		result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_8_4_3
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	misraResults, err := rule_17_4.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}

	results := generateMisraResult(misraResults)
	return results, nil
}
