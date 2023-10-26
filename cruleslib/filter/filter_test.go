/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package filter

import (
	"path/filepath"
	"reflect"
	"testing"

	pb "naive.systems/analyzer/analyzer/proto"
)

func TestGetRuleNameFromErrorMessage(t *testing.T) {
	for _, testCase := range [...]struct {
		expectedRule string
		msg          string
	}{
		{
			expectedRule: "misra_c_2012/dir_4_11",
			msg:          "[C5519][misra-c2012-dir-4.11]: 不修改的变量必须使用const修饰",
		}, {
			expectedRule: "misra_c_2012/rule_13_2",
			msg:          "[C5519][misra-c2012-13.2]: 不修改的变量必须使用const修饰",
		},
		{
			expectedRule: "misra_cpp_2008/rule_7_1_1",
			msg:          "[CXX5548][misra-cpp2008-7.1.1]: 不修改的变量必须使用const修饰",
		},
		{
			expectedRule: "gjb5369/rule_13_2_1",
			msg:          "[C7819][gjb-5369-13.2.1]: 不修改的变量必须使用const修饰",
		},
		{
			expectedRule: "gjb8114/rule_13_2_1",
			msg:          "[C7819][gjb-8114-13.2.1]: 不修改的变量必须使用const修饰",
		},
		{
			expectedRule: "gjb8114/rule_A_13_2_1",
			msg:          "[C7819][gjb-8114-A.13.2.1]: 不修改的变量必须使用const修饰",
		},
	} {
		t.Run(testCase.expectedRule, func(t *testing.T) {
			rule, _ := GetRuleNameFromErrorMessage(testCase.msg)
			if !reflect.DeepEqual(rule, testCase.expectedRule) {
				t.Errorf("unexpected result for %v. got: %v. expected: %v.", testCase.msg, rule, testCase.expectedRule)
			}
		})
	}
}

func TestDeleteResultsWithCertainSuffixs(t *testing.T) {
	var testCase pb.ResultsList
	var suffixs []string
	suffixs = append(suffixs, ".c")
	notDeleteResult := pb.Result{Path: "notDeleteResult.cpp"}
	toDeleteResult := pb.Result{Path: "toDeleteResult.c"}
	testCase.Results = append(testCase.Results, &notDeleteResult)
	testCase.Results = append(testCase.Results, &toDeleteResult)

	t.Run("TestDeleteResultsWithCertainSuffixs", func(t *testing.T) {
		rule := DeleteResultsWithCertainSuffixs(&testCase, suffixs)
		suffix := make(map[string]struct{})
		for _, str := range suffixs {
			suffix[str] = struct{}{}
		}
		for _, rtn := range rule.Results {
			if _, ok := suffix[filepath.Ext(rtn.Path)]; ok {
				t.Errorf("found %v in returned ResultsLust, which is expected to be deleted", rtn.Path)
			}
		}
	})

}
