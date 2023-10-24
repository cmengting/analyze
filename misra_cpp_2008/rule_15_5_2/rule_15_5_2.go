/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_5_2

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reportJson, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_cxx_2008.ExceptionThrow", opts)
	if err != nil {
		glog.Errorf("failed to execute csa: %v", err)
		return nil, err
	}
	resultsList := &pb.ResultsList{}
	for _, run := range reportJson.Runs {
		for _, resultsContent := range run.Results {
			if resultsContent.RuleId == "misra_cxx_2008.ExceptionThrow" {
				result := &pb.Result{}
				result.Path = strings.Replace(resultsContent.Locations[0].PhysicalLocation.ArtifactLocation.Uri, "file://", "", 1)
				result.LineNumber = resultsContent.Locations[0].PhysicalLocation.Region.StartLine
				result.ErrorMessage = "当一个函数的声明包含一个异常说明时，该函数只得抛出指定类型的异常"
				result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_15_5_2
				resultsList.Results = append(resultsList.Results, result)
			}
		}
	}
	return resultsList, nil
}
