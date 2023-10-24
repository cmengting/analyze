/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package main

import (
	"encoding/json"
	"flag"
	"fmt"

	"github.com/golang/glog"
	"google.golang.org/protobuf/encoding/protojson"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/misra/analyzer/analyzerinterface"
	"naive.systems/analyzer/misra/checker_integration/csa"
	"naive.systems/analyzer/misra/checker_integration/misra"
)

func main() {
	compileCommandsPath := flag.String("compile_commands_path", "", "Absolute path to the compile_commands file")
	checkRules := flag.String("check_rules", "", "Rule numbers to be analyzed, like misra_c_2012/rule_22_1, misra_c_2012/rule_22_2")
	resultsDir := flag.String("results_dir", "/output/", "Absolute path to the directory of results files")
	checkerConfig := flag.String("checker_config",
		`{"csa_system_lib_options":"-isystem /usr/lib/gcc/x86_64-redhat-linux/11/include/",
		  "infer_bin":"infer",
		  "clang_bin":"bazel-bin/external/llvm-project/clang/clang",
		  "code_checker_bin":"CodeChecker",
		  "cppcheck_bin":"third_party/cppcheck/cppcheck",
		  "python_bin":"python3",
		  "clangtidy_bin":"clang-tidy",
		  "clangquery_bin":"clang-query",
		  "num_workers":14,
		  "infer_extra_options":"--jobs=8 --max-nesting=3 --bo-field-depth-limit=6"}`,
		"Checker configuration in JSON format")
	var ignoreDirPatterns analyzerinterface.ArrayFlags
	flag.Var(&ignoreDirPatterns, "ignore_dir", "Shell file name pattern to a directory that will be ignored")
	flag.Parse()
	defer glog.Flush()
	glog.Info("checkerConfig", *checkerConfig)
	parsedCheckerConfig := &pb.CheckerConfiguration{}
	err := protojson.Unmarshal([]byte(*checkerConfig), parsedCheckerConfig)
	if err != nil {
		glog.Fatal("parsing checker config: ", err)
	}
	buildActions, err := csa.ParseCompileCommands(
		*compileCommandsPath,
		ignoreDirPatterns,
		parsedCheckerConfig,
		true,
		/*onlyCppcheck=*/ false,
		/*isDev*/ true,
	)
	if err != nil {
		glog.Errorf("checker_integration.csa.ParseCompileCommands: %v", err)
	}
	resultsList, err := misra.Exec("/src", *compileCommandsPath, buildActions, *checkRules, *resultsDir, parsedCheckerConfig, false, 30, 90)
	if err != nil {
		glog.Errorf("checker_integration/misra/cmd/main.misra.Exec: %v", err)
	}
	for _, result := range resultsList.Results {
		resultBytes, err := json.Marshal(result)
		if err != nil {
			glog.Errorf("checker_integration/misra/cmd/main.json.Marshal: %v", err)
		}
		fmt.Println(string(resultBytes))
	}
}
