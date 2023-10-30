/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzer

import (
	"testing"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/misra/checker_integration/checkrule"
)

func TestResolveCheckerBinariesPath(t *testing.T) {
	for _, testCase := range [...]struct {
		name              string
		binPath           string
		checkRules        []checkrule.CheckRule
		enableCodeChecker bool
		expectedErr       error
	}{
		{
			name:    "skip CodeChecker",
			binPath: "CodeChecker",
			checkRules: []checkrule.CheckRule{
				{
					Name:        "misra_c_2012/dir_4_14",
					JSONOptions: checkrule.JSONOption{},
				},
			},
			enableCodeChecker: false,
			expectedErr:       nil,
		},
	} {
		t.Run(testCase.name, func(t *testing.T) {
			config := &pb.CheckerConfiguration{}
			config.InferBin = testCase.binPath
			config.ClangBin = testCase.binPath
			config.CodeCheckerBin = testCase.binPath
			config.CppcheckBin = testCase.binPath
			config.PythonBin = testCase.binPath
			config.ClangtidyBin = testCase.binPath
			config.ClangqueryBin = testCase.binPath
			config.ClangmappingBin = testCase.binPath
			err := resolveCheckerBinariesPath(testCase.checkRules, config, testCase.enableCodeChecker)
			if err != testCase.expectedErr {
				t.Errorf("unexpected error for resolving binary binaries. error: %v. expected: %v.", err, testCase.checkRules)
			}
		})
	}
}
