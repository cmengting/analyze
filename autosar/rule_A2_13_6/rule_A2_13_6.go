/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package rule_A2_13_6

import (
	"bufio"
	"os"
	"regexp"
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/misra/analyzer"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	results := &pb.ResultsList{}
	compileCommandsPath := options.GetCompileCommandsPath(srcdir)
	sourceFiles, err := analyzer.ListSourceFiles(compileCommandsPath, opts.EnvOption.IgnoreDirPatterns)
	if err != nil {
		glog.Errorf("failed to get source files: %v", err)
	}

	rMultilineComment := regexp.MustCompile(`(?s:/\*.*?\*/)`)
	rComment := regexp.MustCompile(`(?m://.*$)`)
	rStringLiteral := regexp.MustCompile(`(?s:".*?")`)
	rCharLiteral := regexp.MustCompile(`'.*'`)

	for _, sourceFile := range sourceFiles {
		content := ""
		file, _ := os.Open(sourceFile)
		defer file.Close()

		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			content += line + "\n"
		}

		content = delete(content, rMultilineComment, rComment, rStringLiteral, rCharLiteral)

		lines := strings.Split(content, "\n")
		for n, line := range lines {
			if strings.Contains(line, "\\U") || strings.Contains(line, "\\u") {
				reportError(n+1, sourceFile, results)
			}
		}
	}
	return results, nil
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
		ErrorMessage: "Universal character names shall be used only inside character or string literals.",
	})
}
