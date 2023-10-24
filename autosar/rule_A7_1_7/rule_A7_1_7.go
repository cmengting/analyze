/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A7_1_7

import (
	"bufio"
	"os"
	"regexp"
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/analyzer"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results, err := runner.RunLibtooling(srcdir, "autosar/rule_A7_1_7", checker_integration.Libtooling_STU, opts)
	if err != nil {
		return nil, err
	}

	compileCommandsPath := options.GetCompileCommandsPath(srcdir)
	sourceFiles, err := analyzer.ListSourceFiles(compileCommandsPath, opts.EnvOption.IgnoreDirPatterns)
	if err != nil {
		glog.Errorf("failed to get source files: %v", err)
	}

	rMultilineComment := regexp.MustCompile(`(?s:/\*.*?\*/)`)
	rComment := regexp.MustCompile(`(?m://.*$)`)
	rStringLiteral := regexp.MustCompile(`(?s:".*?")`)
	rCharLiteral := regexp.MustCompile(`'.*?'`)
	rForStmt := regexp.MustCompile(`for\s*?\(([^\(\)]*?(\([^\(\)]*?\))*?[^\(\)]*?)*?\)`)

	for _, sourceFile := range sourceFiles {
		content := ""
		file, _ := os.Open(sourceFile)
		defer file.Close()

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			content += line + "\n"
		}

		content = delete(content, rMultilineComment, rComment, rStringLiteral, rCharLiteral, rForStmt)

		lines := strings.Split(content, "\n")
		for n, line := range lines {
			if strings.Count(line, ";") > 1 {
				reportError(n+1, sourceFile, results)
			}
		}
	}
	return runner.RemoveDup(results), err
}

func delete(str string, rList ...*regexp.Regexp) string {
	for _, r := range rList {
		str = r.ReplaceAllLiteralString(str, "")
	}
	return str
}

func reportError(lineNumber int, filepath string, results *pb.ResultsList) {
	results.Results = append(results.Results, &pb.Result{
		Path:         filepath,
		LineNumber:   int32(lineNumber),
		ErrorMessage: "Each expression statement and identifier declaration shall be placed on a separate line.",
	})
}
