/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_0_3_2

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra_c_2012_crules/dir_4_7"
)

func generateMisraResult(resultsOfMisra *pb.ResultsList) *pb.ResultsList {
	results := &pb.ResultsList{}
	for _, rs := range resultsOfMisra.Results {
		results.Results = append(results.Results, &pb.Result{
			Path:         rs.Path,
			LineNumber:   rs.LineNumber,
			ErrorMessage: "如果函数返回了错误信息，那么必须检测该错误信息",
			ErrorKind:    pb.Result_MISRA_CPP_2008_RULE_0_3_2,
		})
	}
	return results
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	misraResults, err := dir_4_7.Analyze(srcdir, opts)
	if err != nil {
		return nil, err
	}
	results := generateMisraResult(misraResults)

	return results, nil
}
