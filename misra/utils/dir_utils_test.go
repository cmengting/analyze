/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package utils

import (
	"io/fs"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"github.com/golang/glog"
)

var testDir = "./simple_test"

func TestMain(m *testing.M) {
	err := os.MkdirAll(testDir, os.ModePerm)
	if err != nil {
		glog.Errorf("test setUp: %v", err)
	}
	code := m.Run()
	// clear created dirs and files in testDir after tesing
	err = os.RemoveAll(testDir)
	if err != nil {
		glog.Errorf("test tearDown: %v", err)
	}
	os.Exit(code)
}

// clean the logs folder in testDir
func TestCleanCache1(t *testing.T) {
	err := os.MkdirAll(filepath.Join(testDir, "logs"), os.ModePerm)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}

	err = CleanCache(testDir, nil)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}

	testD, err := os.ReadDir(testDir)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	expectD := []fs.DirEntry{}
	if !reflect.DeepEqual(testD, expectD) {
		t.Errorf("unexpected result. Get %v, Expect %v", testD, expectD)
	}
}

// ignore the logs folder and clean other entities
func TestCleanCache2(t *testing.T) {
	err := os.MkdirAll(filepath.Join(testDir, "logs"), os.ModePerm)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	err = os.MkdirAll(filepath.Join(testDir, "temp"), os.ModePerm)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	_, err = os.Create(filepath.Join(testDir, "test"))
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	testD, err := os.ReadDir(testDir)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	expectD := []fs.DirEntry{}
	for _, d := range testD {
		if d.Name() == "logs" {
			expectD = append(expectD, d)
		}
	}

	err = CleanCache(testDir, []string{"logs"})
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}

	testD, err = os.ReadDir(testDir)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	if !reflect.DeepEqual(testD, expectD) {
		t.Errorf("unexpected result. Get %v, Expect %v", testD, expectD)
	}
}

// ignore the results file and clean other entities including
// files named with results in sub-directories
func TestCleanCache3(t *testing.T) {
	err := os.MkdirAll(filepath.Join(testDir, "logs"), os.ModePerm)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	_, err = os.Create(filepath.Join(filepath.Join(testDir, "logs"), "results"))
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	_, err = os.Create(filepath.Join(testDir, "results"))
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	testD, err := os.ReadDir(testDir)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	expectD := []fs.DirEntry{}
	for _, d := range testD {
		if d.Name() == "results" {
			expectD = append(expectD, d)
		}
	}

	err = CleanCache(testDir, []string{"results"})
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}

	testD, err = os.ReadDir(testDir)
	if err != nil {
		t.Errorf("unexpected error: %v", err)
	}
	if !reflect.DeepEqual(testD, expectD) {
		t.Errorf("unexpected result. Get %v, Expect %v", testD, expectD)
	}
}
