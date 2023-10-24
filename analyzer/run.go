/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzer

import (
	"fmt"
	"os"
	"path/filepath"

	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"
	"naive.systems/analyzer/analyzer/podman"
	pb "naive.systems/analyzer/analyzer/proto"
)

type Analyzer struct {
	Definition *pb.Definition
	Rules      []*Rule
}

type Rule struct {
	Name        string
	JSONOptions string
}

func Init(definitionConfigPath string) (*pb.Definitions, error) {
	content, err := os.ReadFile(definitionConfigPath)
	if err != nil {
		return nil, err
	}
	var definitions pb.Definitions
	err = prototext.Unmarshal(content, &definitions)
	if err != nil {
		return nil, err
	}
	return &definitions, nil
}

func RunSingleAnalyzer(
	podmanBinPath,
	workingDir,
	logDir,
	imageName string,
	analyzer *Analyzer,
	checkerConfig *pb.CheckerConfiguration,
	srcDir string,
	ignorePatterns []string,
	projectType,
	qtProPath,
	scriptContents,
	projectName,
	checkRulesDir string,
) (*pb.ResultsList, error) {
	invocationPath := analyzer.Definition.InvocationPath
	err := podman.Run(
		podmanBinPath,
		checkRulesDir,
		workingDir,
		logDir,
		imageName,
		invocationPath,
		checkerConfig,
		srcDir,
		ignorePatterns,
		projectType,
		qtProPath,
		scriptContents,
		projectName,
	)
	if err != nil {
		return nil, fmt.Errorf("podman.Run: %v", err)
	}
	resultList, err := getResult(filepath.Join(logDir, "results"))
	if err != nil {
		resultList, err = getResult(filepath.Join(logDir, "results.nsa_results"))
		if err != nil {
			return nil, fmt.Errorf("getResult(%s): %v", filepath.Join(logDir, "results"), err)
		}
	}
	return resultList, nil
}

func getResult(resultFile string) (*pb.ResultsList, error) {
	content, err := os.ReadFile(resultFile)
	if err != nil {
		return nil, err
	}
	var result pb.ResultsList
	err = proto.Unmarshal(content, &result)
	if err != nil {
		return nil, err
	}
	return &result, nil
}
