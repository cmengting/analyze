/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_M5_2_9

import (
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	mc "naive.systems/analyzer/misra_cpp_2008/rule_5_2_9"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	return mc.Analyze(srcdir, opts)
}
