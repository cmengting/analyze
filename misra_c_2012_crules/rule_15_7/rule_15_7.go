/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_15_7

import (
	"fmt"
	"strconv"
	"strings"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

type ErrResult struct {
	FileName   string
	LineNumber int32
}

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Libtooling cannot handle the empty {} with comments, it's just auxiliary.
	// Gives the pair (expansion -> spelling location)
	libtooling_results, err := runner.RunLibtooling(srcdir, "misra/rule_15_7", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}

	// Build the map from expanding loc to spelling loc
	expansion_to_spelling_loc_map := make(map[string]string)
	for _, v := range libtooling_results.Results {
		if expansionLoc := fmt.Sprint(v.Path, ":", v.LineNumber); expansionLoc != v.ErrorMessage {
			expansion_to_spelling_loc_map[expansionLoc] = v.ErrorMessage
		}
	}

	// Every report from cppcheck is true positive, but we need adjust the line to spelling location
	results, err := runner.RunCppcheck(srcdir, "misra_c_2012/rule_15_7", checker_integration.Cppcheck_STU, opts)
	if err != nil {
		return nil, err
	}

	// Map the err line by expansion_to_spelling_loc_map
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C1801][misra-c2012-15.7]") {
			v.ErrorMessage = "[C1801][misra-c2012-15.7]: 所有if … else if构造都必须以一个else语句终止"
			reportLoc := fmt.Sprint(v.Path, ":", v.LineNumber)
			spellingLoc, existence := expansion_to_spelling_loc_map[reportLoc]
			//Existence means expanding from a spelling, we map it by expansion_to_spelling_loc_map
			//Otherwise, it's not a spelling, we report without modification
			if existence {
				i := strings.LastIndex(spellingLoc, ":")
				spellingLine, err := strconv.ParseInt(spellingLoc[i+1:], 10, 32)
				if err != nil {
					return nil, err
				}
				v.LineNumber = int32(spellingLine)
				v.Path = spellingLoc[:i]
			}
		}
	}

	return results, nil
}
