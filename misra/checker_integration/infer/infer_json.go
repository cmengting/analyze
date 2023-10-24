/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package infer

type TraceItem struct {
	Level        int32  `json:"level"`
	Filename     string `json:"filename"`
	LineNumber   int32  `json:"line_number"`
	ColumnNumber int32  `json:"column_number"`
	Description  string `json:"description"`
}

type InferReport struct {
	Bug_type  string      `json:"bug_type"`
	Qualifier string      `json:"qualifier"`
	Line      int32       `json:"line"`
	File      string      `json:"file"`
	Bug_trace []TraceItem `json:"bug_trace"`
}
