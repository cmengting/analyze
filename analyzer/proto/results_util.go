/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package proto

type resultBlood struct {
	path         string
	lineNumber   int32
	errorMessage string
}

// ResultsSet is an alternative to ResultsList. When Add() is called, it checks
// ResultBlood to identify unique Results. It preserves Results' adding order.
type ResultsSet struct {
	// You can manipulate ResultsList beyond the limits.
	ResultsList
	stored map[resultBlood]struct{}
}

func NewResultsSet() *ResultsSet {
	set := ResultsSet{}
	set.stored = make(map[resultBlood]struct{})
	return &set
}

func NewResultsSetFromList(list *ResultsList) *ResultsSet {
	set := NewResultsSet()
	set.AddList(list)
	return set
}

func (rs *ResultsSet) Add(r *Result) {
	blood := resultBlood{
		path:         r.Path,
		lineNumber:   r.LineNumber,
		errorMessage: r.ErrorMessage,
	}
	if _, reported := rs.stored[blood]; !reported {
		rs.stored[blood] = struct{}{}
		rs.Results = append(rs.Results, r)
	}
}

func (rs *ResultsSet) AddList(list *ResultsList) {
	for _, r := range list.Results {
		rs.Add(r)
	}
}
