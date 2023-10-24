/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_2_7

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func MisraCXXCheckRule5_2_7(reportJson *csa.CSAReport) *pb.ResultsList {
	resultsList := &pb.ResultsList{}
	for _, run := range reportJson.Runs {
		for _, resultsContent := range run.Results {
			if resultsContent.RuleId == "misra_cxx_2008.PointerCast" {
				result := &pb.Result{}
				result.Path = strings.Replace(resultsContent.Locations[0].PhysicalLocation.ArtifactLocation.Uri, "file://", "", 1)
				result.LineNumber = resultsContent.Locations[0].PhysicalLocation.Region.StartLine
				result.ErrorMessage = "一个具有指针类型的对象不得被直接或间接转换为不相关的指针类型"
				result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_2_7
				resultsList.Results = append(resultsList.Results, result)
				glog.Info(result.ErrorMessage)
			}
		}
	}
	return resultsList
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_cxx_2008.PointerCast", opts)
	if err != nil {
		glog.Errorf("failed to execute libtooling: %v", err)
		return nil, err
	}
	return MisraCXXCheckRule5_2_7(results), nil
}
