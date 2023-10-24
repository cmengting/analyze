/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package csa

type ArtifactLocation struct {
	Uri string `json:"uri"`
}

type Region struct {
	StartLine int32 `json:"startLine"`
}

type PhysicalLocation struct {
	ArtifactLocation ArtifactLocation `json:"artifactLocation"`
	Region           Region           `json:"region"`
}

type Message struct {
	Text string `json:"text"`
}

type LocationsContent struct {
	PhysicalLocation PhysicalLocation `json:"physicalLocation"`
}

type ResultsContent struct {
	RuleId    string             `json:"ruleId"`
	Message   Message            `json:"message"`
	Locations []LocationsContent `json:"locations"`
}

type RunsContent struct {
	Results []ResultsContent `json:"results"`
}

type CSAReport struct {
	Runs []RunsContent `json:"runs"`
}
