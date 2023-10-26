/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzerinterface

import (
	"errors"
	"testing"
)

func TestMatchIgnoreDirPatterns(t *testing.T) {
	for _, testCase := range [...]struct {
		name           string
		ignorePatterns []string
		filePath       string
		expectedResult bool
		expectedErr    error
	}{
		{
			name:           "match file in the same folder",
			ignorePatterns: []string{"/src/test/**/*"},
			filePath:       "/src/test/test1.c",
			expectedResult: true,
			expectedErr:    nil,
		},
		{
			name:           "match file in the recursive folder",
			ignorePatterns: []string{"/src/test/**/*"},
			filePath:       "/src/test/test1.c",
			expectedResult: true,
			expectedErr:    nil,
		},
		{
			name:           "no matched file",
			ignorePatterns: []string{"/src/core/**/*"},
			filePath:       "/src/test/test1.c",
			expectedResult: false,
			expectedErr:    nil,
		},
		{
			name:           "invalid pattern",
			ignorePatterns: []string{"/src/[**/"},
			filePath:       "/src/test/test1.c",
			expectedResult: false,
			expectedErr:    errors.New("malformed ignore_dir pattern /src/[**/"),
		},
	} {
		t.Run(testCase.name, func(t *testing.T) {
			matched, err := MatchIgnoreDirPatterns(testCase.ignorePatterns, testCase.filePath)
			if err != nil || testCase.expectedErr != nil {
				if err.Error() != testCase.expectedErr.Error() {
					t.Errorf("unexpected result for test %v. error: %v. expected: %v.", testCase.name, err, testCase.expectedErr)
				}
			} else if matched != testCase.expectedResult {
				t.Errorf("unexpected result for test %v. result: %v. expected: %v.", testCase.name, matched, testCase.expectedResult)
			}
		})
	}
}
