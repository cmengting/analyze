/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package misra

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"

	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/misra/analyzer/analyzerinterface"
	"naive.systems/analyzer/misra/checker_integration/infer"
)

// Get CallGraph of source files
func GetImpureFunctions(srcDir, compileCommandsPath string, resultsDir string, config *pb.CheckerConfiguration, limitMemory bool, timeoutNormal, timeoutOom int) ([]string, error) {
	plugin := "--impurity-only --function-pointer-specialization"
	filteredCompileCommandsFolder, err := analyzerinterface.CreateTempFolderContainsFilteredCompileCommandsJsonFile(compileCommandsPath)
	if err != nil {
		return nil, fmt.Errorf("analyzerinterface.CreateTempFolderContainsFilteredCompileCommandsJsonFile: %v", err)
	}
	defer os.RemoveAll(filteredCompileCommandsFolder)
	reportJson, err := infer.GetInferReport(srcDir, filepath.Join(filteredCompileCommandsFolder, analyzerinterface.CCJson), plugin, resultsDir, config, limitMemory, timeoutNormal, timeoutOom)
	if err != nil {
		return nil, err
	}
	re := regexp.MustCompile(`Impure function (.*).`)
	impureFunctions := make([]string, 0)
	for _, report := range reportJson {
		if report.Bug_type == "IMPURE_FUNCTION" {
			match := re.FindStringSubmatch(report.Qualifier)
			impureFunctions = append(impureFunctions, match[1])
		}
	}
	return impureFunctions, nil
}
