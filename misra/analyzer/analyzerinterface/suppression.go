/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzerinterface

import (
	"os"
	"path/filepath"

	"google.golang.org/protobuf/encoding/protojson"
	pb "naive.systems/analyzer/analyzer/proto"
)

type suppressionAsKey struct {
	content  string
	ruleCode string
}

func visit(files *[]string) filepath.WalkFunc {
	return func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() || filepath.Ext(path) != ".nsa_suppression" {
			return nil
		}
		*files = append(*files, path)
		return nil
	}
}

func getSuppressionMap(suppressionFiles []string) (map[suppressionAsKey]*pb.Suppression, error) {
	suppressionMap := make(map[suppressionAsKey]*pb.Suppression)
	for _, suppressionFile := range suppressionFiles {
		bytes, err := os.ReadFile(suppressionFile)
		suppressions := &pb.SuppressionsList{}
		if err != nil {
			return nil, err
		}
		err = protojson.Unmarshal(bytes, suppressions)
		if err != nil {
			return nil, err
		}
		for _, suppression := range suppressions.Suppressions {
			key := suppressionAsKey{content: suppression.Content, ruleCode: suppression.RuleCode}
			suppressionMap[key] = suppression
		}
	}
	return suppressionMap, nil
}
