/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A3_1_3

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func checkFileNameExtension(buildActions *[]csa.BuildAction) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	for _, action := range *buildActions {
		path := action.Command.File
		if !strings.HasSuffix(path, ".cpp") {
			results.Results = append(results.Results, &pb.Result{
				Path:         path,
				ErrorMessage: "Implementation files, that are defined locally in the project, should have a file name extension of \".cpp\".",
			})
		}
	}
	return results, nil
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := checkFileNameExtension(opts.EnvOption.BuildActions)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	return results, nil
}
