/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A3_1_2

import (
	"bufio"
	"os"
	"regexp"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func checkHeader(buildActions *[]csa.BuildAction) (*pb.ResultsList, error) {
	resultsList := &pb.ResultsList{}

	for _, action := range *buildActions {
		path := action.Command.File
		file, err := os.Open(path)
		if err != nil {
			glog.Errorf("failed to open source file: %s, error: %v", path, err)
			return nil, err
		}

		scanner := bufio.NewScanner(file)
		lineno := 0

		regex := regexp.MustCompile(`^#include`)
		headersRegex := regexp.MustCompile(`"\w*\.(h|hpp|hxx)"`)
		for scanner.Scan() {
			line := scanner.Text()
			lineno++
			if regex.MatchString(line) {
				if !headersRegex.MatchString(line) {
					resultsList.Results = append(resultsList.Results, &pb.Result{
						Path:         path,
						LineNumber:   int32(lineno),
						ErrorMessage: "Header files, that are defined locally in the project, shall have a file name extension of one of: \".h\", \".hpp\" or \".hxx\".",
					})
				}
			}
		}
		err = file.Close()
		if err != nil {
			return nil, err
		}
	}

	return resultsList, nil
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := checkHeader(opts.EnvOption.BuildActions)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	return results, nil
}
