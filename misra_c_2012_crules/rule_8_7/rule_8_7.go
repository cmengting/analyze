/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_8_7

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "misra/rule_8_7", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	finalResult := &pb.ResultsList{}
	for _, v := range results.Results {
		firstDeclPath := v.OtherFilename
		// Do not report if the first declaration is not in the file under srcdir
		if !strings.HasPrefix(firstDeclPath, strings.TrimSuffix(srcdir, "/")+"/") {
			continue
		}
		if strings.HasPrefix(v.ErrorMessage, "[C0508][misra-c2012-8.7]") {
			v.ErrorMessage = "[C0508][misra-c2012-8.7]: 不应使用外部链接定义仅在一个翻译单元中被引用的函数和对象"
		}
		finalResult.Results = append(finalResult.Results, v)
	}
	return finalResult, nil
}
