/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_1

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra/rule_5_1", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	// rewrite error messages
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1109][misra-c2012-5.1]: contain non-ASCII characters") {
			v.ErrorMessage = "[C1109][misra-c2012-5.1]: 包含非ASCII字符"
		} else if strings.HasPrefix(v.ErrorMessage, "[C1109][misra-c2012-5.1]: violation of misra-c2012-5.1") {
			v.ErrorMessage = "[C1109][misra-c2012-5.1]: 不得使用重名的外部标识符"
		}
	}
	return results, nil
}
