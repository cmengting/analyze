/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A18_5_3

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"strings"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reportJson, err := runner.RunCSA(srcdir, "-analyzer-checker=unix.MismatchedDeallocator", opts)
	if err != nil {
		glog.Errorf("failed to execute csa: %v", err)
		return nil, err
	}
	resultsList := &pb.ResultsList{}
	for _, run := range reportJson.Runs {
		for _, resultsContent := range run.Results {
			if resultsContent.RuleId == "unix.MismatchedDeallocator" {
				result := &pb.Result{}
				result.Path = strings.Replace(resultsContent.Locations[0].PhysicalLocation.ArtifactLocation.Uri, "file://", "", 1)
				result.LineNumber = resultsContent.Locations[0].PhysicalLocation.Region.StartLine
				result.ErrorMessage = "The form of the delete expression shall match the form of the new expression used to allocate the memory."
				resultsList.Results = append(resultsList.Results, result)
				glog.Info(result.ErrorMessage)
			}
		}
	}
	return resultsList, nil
}
