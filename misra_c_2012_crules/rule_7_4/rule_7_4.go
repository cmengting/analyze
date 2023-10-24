/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_7_4

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// libtooling
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_7_4", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	// misra
	misraResults := runner.RunMisra(srcdir, "misra_c_2012/rule_7_4", opts)
	// merge
	errMsg := "[C0901][misra-c2012-7.4]: 不得将字符串字面量赋值给对象，除非对象类型为“指向const修饰的char的指针”"
	results := &pb.ResultsList{}
	results.Results = append(results.Results, libtoolingResults.Results...)
	results.Results = append(results.Results, misraResults.Results...)
	for _, result := range results.Results {
		if strings.HasPrefix(result.ErrorMessage, "[C0901][misra-c2012-7.4]") {
			result.ErrorMessage = errMsg
		}
	}
	// remove duplicate
	results = runner.RemoveDup(results)

	return results, nil
}
