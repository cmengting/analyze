/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A23_0_2

import (
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	reportJson, err := runner.RunCSA(srcdir, "-analyzer-checker=alpha.cplusplus.InvalidatedIterator -analyzer-checker=alpha.cplusplus.IteratorRange -analyzer-config aggressive-binary-operation-simplification=true", opts)
	if err != nil {
		return nil, err
	}
	results := &pb.ResultsList{}
	for _, run := range reportJson.Runs {
		for _, resultsContent := range run.Results {
			if resultsContent.RuleId == "alpha.cplusplus.InvalidatedIterator" ||
				resultsContent.RuleId == "alpha.cplusplus.IteratorRange" {
				result := &pb.Result{}
				result.Path = strings.Replace(resultsContent.Locations[0].PhysicalLocation.ArtifactLocation.Uri, "file://", "", 1)
				result.LineNumber = resultsContent.Locations[0].PhysicalLocation.Region.StartLine
				result.ErrorMessage = "Elements of a container shall only be accessed via valid references, iterators, and pointers."
				results.Results = append(results.Results, result)
			}
		}
	}
	return results, nil
}
