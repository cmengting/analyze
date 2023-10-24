/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package utils

import (
	"bufio"
	"fmt"
	"os/exec"
	"regexp"
	"strings"

	"github.com/golang/glog"
)

// Returns the stdout of a command as string.
func GetCommandStdout(commands []string, workingDir string) ([]string, error) {
	var stdLogs []string
	cmd := exec.Command(commands[0], commands[1:]...)
	if workingDir != "" {
		cmd.Dir = workingDir
	}
	stdoutStderr, err := cmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("cmd.CombinedOutput: %v", err)
	}
	stdLogs = strings.Fields(string(stdoutStderr))
	return stdLogs, nil
}

// Returns the lines of stdout of a command as string.
func GetCommandStdoutLines(cmd *exec.Cmd, workingDir string) ([]string, error) {
	var stdLogs []string
	if workingDir != "" {
		cmd.Dir = workingDir
	}
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, fmt.Errorf("cmd.StdoutPipe: %v", err)
	}
	if err := cmd.Start(); err != nil {
		return nil, fmt.Errorf("cmd.Start: %v", err)
	}
	in := bufio.NewScanner(stdout)
	for in.Scan() {
		stdLogs = append(stdLogs, string(in.Text()))
	}
	if err := in.Err(); err != nil {
		return stdLogs, fmt.Errorf("bufio.NewScanner: %v", err)
	}
	return stdLogs, nil
}

func GetCurrentClangVersion() string {
	stdoutLines, err := GetCommandStdoutLines(exec.Command("clang", "--version"), ".")
	if err != nil {
		glog.Errorf("getCommandStdoutLines: %v", err)
		return ""
	}
	clangVersionPattern := "(?P<vendor>clang|Apple LLVM) version (?P<clang_version>[^\\s]*)"
	reg := regexp.MustCompile(clangVersionPattern)
	result := reg.FindAllStringSubmatch(strings.Join(stdoutLines, " "), -1)
	return result[0][2]
}

func GetCurrentCppVersion() string {
	stdoutLines, err := GetCommandStdoutLines(exec.Command("c++", "--version"), ".")
	if err != nil {
		glog.Errorf("getCommandStdoutLines: %v", err)
		return ""
	}
	cppVersionPattern := "c\\+\\+ \\((?P<vendor>clang|GCC)\\) (?P<cpp_version>[^\\s]*)"
	reg := regexp.MustCompile(cppVersionPattern)
	result := reg.FindAllStringSubmatch(strings.Join(stdoutLines, " "), -1)
	return strings.Split(result[0][2], ".")[0]
}
