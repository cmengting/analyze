/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package proto

import (
	"testing"
)

func TestResultsSet(t *testing.T) {
	set := NewResultsSet()
	set.Add(&Result{
		Path:         "file_a",
		LineNumber:   2,
		ErrorMessage: "error_a",
	})
	set.Add(&Result{
		Path:         "file_a",
		LineNumber:   2,
		ErrorMessage: "error_a",
	})
	set.Add(&Result{
		Path:         "file_a",
		LineNumber:   2,
		ErrorMessage: "error_b",
	})
	if len(set.Results) != 2 {
		t.Fatalf("ResultsSet is not a set, expect size: 2, actual: %d", len(set.Results))
	}
}

func TestResultsSetFromList(t *testing.T) {
	list := &ResultsList{Results: []*Result{
		{Path: "file_a", LineNumber: 2, ErrorMessage: "error_a"},
		{Path: "file_a", LineNumber: 2, ErrorMessage: "error_a"},
		{Path: "file_a", LineNumber: 2, ErrorMessage: "error_b"},
	}}
	set := NewResultsSetFromList(list)
	if len(set.Results) != 2 {
		t.Fatalf("ResultsSetFromList is not a set, expect size: 2, actual: %d", len(set.Results))
	}
}
