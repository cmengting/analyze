/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzer

import (
	"os"
	"testing"

	pb "naive.systems/analyzer/analyzer/proto"

	"google.golang.org/protobuf/encoding/prototext"
)

func TestLoadCheckerConfigFromExample(t *testing.T) {
	path := "analyzer_definitions.textproto.example"
	contents, err := os.ReadFile(path)
	if err != nil {
		t.Error(err)
	}
	defs := pb.Definitions{}
	err = prototext.Unmarshal(contents, &defs)
	if err != nil {
		t.Error(err)
	}
	if defs.CheckerConfig.InferBin != "infer" {
		t.Error(defs.CheckerConfig.InferBin)
	}
}
