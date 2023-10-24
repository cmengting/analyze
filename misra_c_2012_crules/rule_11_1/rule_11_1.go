/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_11_1

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	MisraResults := runner.RunMisra(srcdir, "misra_c_2012/rule_11_1", opts)

	LibtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_11_1", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	results := &pb.ResultsList{}
	results.Results = append(results.Results, MisraResults.Results...)
	results.Results = append(results.Results, LibtoolingResults.Results...)

	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1409][misra-c2012-11.1]") {
			v.ErrorMessage = "[C1409][misra-c2012-11.1]: 指向函数的指针只能与指向函数的指针相互转换，且指针与函数的类型必须兼容"
		}
	}

	results = runner.RemoveDup(results)

	return runner.RemoveDup(results), nil
}
