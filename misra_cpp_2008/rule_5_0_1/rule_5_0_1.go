/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_5_0_1

import (
	"fmt"
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

// TODO:
// llvm 16 will use c++17 as default standard, but the order of
// evalutaion is much stricter, false-positive may occur.
// reference (rule 14-21): https://en.cppreference.com/w/cpp/language/eval_order
func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	var err error
	resultList := &pb.ResultsList{}
	if opts.JsonOption.AggressiveMode != nil && *opts.JsonOption.AggressiveMode {
		resultList, err = runner.RunLibtooling(srcdir, "misra/rule_13_2", checker_integration.Libtooling_CTU, opts)
		if err != nil {
			glog.Error(err)
			return nil, err
		}
		for _, result := range resultList.Results {
			result.ErrorMessage = "表达式的值不应因求值顺序而改变"
			result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_0_1
		}
	}
	ds, err := runner.RunClangSema(srcdir, "-Wunsequenced -Wsequence-point", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	for _, diagnostic := range ds {
		if diagnostic.WarningOption == "unsequenced" {
			result := &pb.Result{}
			result.Path = diagnostic.Filename
			result.LineNumber = int32(diagnostic.Line)
			result.ErrorMessage = "表达式的值不应因求值顺序而改变"
			result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_0_1
			result.ExternalMessage = diagnostic.Message
			resultList.Results = append(resultList.Results, result)
			glog.Info(result.ErrorMessage)
		}
	}
	csa_rs, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.VarUnsequencedAccess", opts)
	if err != nil {
		return nil, err
	}
	for _, run := range csa_rs.Runs {
		results := run.Results
		for _, resultsContent := range results {
			if resultsContent.RuleId == "misra_c_2012.VarUnsequencedAccess" {
				result := &pb.Result{}
				result.Path = strings.Replace(resultsContent.Locations[0].PhysicalLocation.ArtifactLocation.Uri, "file://", "", 1)
				result.LineNumber = resultsContent.Locations[0].PhysicalLocation.Region.StartLine
				result.ErrorMessage = "表达式的值不应因求值顺序而改变"
				result.ErrorKind = pb.Result_MISRA_CPP_2008_RULE_5_0_1
				result.ExternalMessage = resultsContent.Message.Text
				resultList.Results = append(resultList.Results, result)
				glog.Info(result.ErrorMessage)
			}
		}
	}
	return runner.RemoveDup(resultList), nil
}
