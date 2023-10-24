/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_1_3

import (
	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	csaReport, err := runner.RunCSA(srcdir, "-analyzer-checker=alpha.security.ArrayBoundV2 -analyzer-checker=core.DivideZero -analyzer-checker=core.NullDereference -analyzer-checker=unix.Malloc -analyzer-checker=alpha.security.ReturnPtrRange -analyzer-checker=core.StackAddressEscape", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	glog.Info(csaReport)
	results := csa.CheckRule1_3(*csaReport)
	return results, nil
}
