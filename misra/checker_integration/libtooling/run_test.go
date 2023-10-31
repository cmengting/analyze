/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package libtooling

import (
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

func TestLibtoolingSTU(t *testing.T) {
	args := []string{
		"--log-dir=/log",
		"-@@@",
		"--results-path=/output/results",
	}
	sourceFiles := []string{
		"sourcefile1.c",
		"sourcefile2.c",
		"sourcefile3.c",
	}
	expected := [][]string{
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile1.c",
		},
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile2.c",
		},
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile3.c",
		},
	}
	actual := generateSTURuns(args, sourceFiles)
	if !reflect.DeepEqual(expected, actual) {
		t.Errorf("unexpected stu results: %v, expected :%v", actual, expected)
	}
}

func TestLibtoolingCTU(t *testing.T) {
	dir, _ := os.Getwd()
	args := []string{
		"--log-dir=/log",
		"-@@@",
		"--results-path=/output/results",
	}
	sourceFiles := []string{
		"sourcefile1.c",
		"sourcefile2.c",
		"sourcefile3.c",
	}
	expected := [][]string{
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			filepath.Join(dir, "sourceFile"),
		},
	}
	actual := generateCTURuns(args, sourceFiles, dir)
	tmpFilePath := actual[0][len(actual[0])-1]
	defer os.Remove(tmpFilePath)
	if !reflect.DeepEqual(expected, actual) {
		t.Errorf("unexpected stu results: %v, expected :%v", actual, expected)
	}
}

func TestRunsAppend(t *testing.T) {
	dir, _ := os.Getwd()
	args := make([]string, 3, 5)
	args[0] = "--log-dir=/log"
	args[1] = "-@@@"
	args[2] = "--results-path=/output/results"

	sourceFiles := []string{
		"sourcefile1.c",
		"sourcefile2.c",
		"sourcefile3.c",
	}

	expectedCTU := [][]string{
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			filepath.Join(dir, "sourceFile"),
		},
	}

	expectedSTU := [][]string{
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile1.c",
		},
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile2.c",
		},
		{
			"--log-dir=/log",
			"-@@@",
			"--results-path=/output/results",
			"sourcefile3.c",
		},
	}

	actualCTURuns := generateCTURuns(args, sourceFiles, dir)
	tmpFilePath := actualCTURuns[0][len(actualCTURuns[0])-1]
	defer os.Remove(tmpFilePath)

	if !reflect.DeepEqual(expectedCTU, actualCTURuns) {
		t.Errorf("unexpected stu results: %v, expected :%v", actualCTURuns, expectedCTU)
	}

	actualSTURuns := generateSTURuns(args, sourceFiles)
	if !reflect.DeepEqual(expectedSTU, actualSTURuns) {
		t.Errorf("unexpected stu results: %v, expected :%v", actualSTURuns, expectedSTU)
	}
}

func TestSkipAssembly(t *testing.T) {
	sourceFiles := []string{
		"sourcefile1.c",
		"sourcefile2.s",
		"sourcefile3.c",
		"sourcefile4.S",
		"sourcefile5.c",
	}

	expected := []string{
		"sourcefile1.c",
		"sourcefile3.c",
		"sourcefile5.c",
	}

	actual := skipAssembly(sourceFiles)
	if !reflect.DeepEqual(expected, actual) {
		t.Errorf("unexpected skip results: %v, expected :%v", actual, expected)
	}
}
