/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_11_6

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	MisraResults := runner.RunMisra(srcdir, "misra_c_2012/rule_11_6", opts)

	LibtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_11_6", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	results := &pb.ResultsList{}
	results.Results = append(results.Results, MisraResults.Results...)
	results.Results = append(results.Results, LibtoolingResults.Results...)

	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1404][misra-c2012-11.6]") {
			v.ErrorMessage = "[C1404][misra-c2012-11.6]: 指向void的指针和算术类型之间不得显式转换"
		}
	}

	results = runner.RemoveDup(results)

	return runner.RemoveDup(results), nil
}
