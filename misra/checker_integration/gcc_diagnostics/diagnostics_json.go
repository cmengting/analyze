/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package gcc_diagnostics

import "encoding/json"

type Location struct {
	Column int    `json:"column"`
	Line   int    `json:"line"`
	File   string `json:"file"`
}

type Diagnostic struct {
	Kind      string `json:"kind"`
	Locations []struct {
		// If you need to read another field of `locations`,
		// just create it like `caret` below.
		// Note that the type of fields of `locations` are not all 'Location'
		// (maybe a field `label` of type 'string').
		Caret Location `json:"caret"`
	} `json:"locations"`
	Option  string `json:"option"`
	Message string `json:"message"`
}

func ParseDiagnosticsJson(output []byte) ([]Diagnostic, error) {
	var diagnostics []Diagnostic
	err := json.Unmarshal(output, &diagnostics)
	return diagnostics, err
}
