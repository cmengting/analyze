/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_13_2

import (
	"fmt"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
	"naive.systems/analyzer/misra/checker_integration/clangsema"
	"naive.systems/analyzer/misra/checker_integration/csa"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	// Clang Static Analyzer
	csaReports, err := runner.RunCSA(srcdir, "-analyzer-checker=misra_c_2012.VarUnsequencedAccess", opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	csaResults := csa.CheckRule13_2(*csaReports)

	// Clangsema
	clangsemaDiagnostics, err := runner.RunClangSema(srcdir, "-Wunsequenced -Wsequence-point", opts)
	if err != nil {
		return nil, fmt.Errorf("clangsema err: %v", err)
	}
	clangsemaResults := clangsema.CheckRule13_2(clangsemaDiagnostics)

	// LibTooling: the order of calling functions may affect the value of the expression
	libtoolingResults, err := runner.RunLibtooling(srcdir, "misra/rule_13_2", checker_integration.Libtooling_CTU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	// Merge differenct checker's results
	results := &pb.ResultsList{}
	results.Results = append(results.Results, csaResults.Results...)
	results.Results = append(results.Results, clangsemaResults.Results...)
	results.Results = append(results.Results, libtoolingResults.Results...)

	// Change the error message
	for _, result := range results.Results {
		result.ErrorMessage = "[C1605][misra-c2012-13.2]: 采用任意允许采用的求值顺序时，表达式的值和表达式的持续的副作用不得发生改变"
	}
	return results, nil
}
