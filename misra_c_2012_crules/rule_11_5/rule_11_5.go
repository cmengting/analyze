/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_11_5

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra/rule_11_5", checker_integration.Libtooling_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.VoidToObjectPtr", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule11_5(*csaReports)
	results.Results = append(results.Results, csaResults.Results...)
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1405][misra-c2012-11.5]") {
			v.ErrorMessage = "[C1405][misra-c2012-11.5]: 指向void的指针不应转换为指向对象的指针"
		}
	}
	return results, nil
}
