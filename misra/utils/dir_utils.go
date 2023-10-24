/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package utils

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/golang/glog"
)

func CleanCache(dir string, filesToIgnore []string) error {
	glog.Info("cleaning cache in ", dir)
	filesSaveMap := make(map[string]bool)
	for _, fileName := range filesToIgnore {
		filesSaveMap[fileName] = true
	}
	dirInfo, err := os.ReadDir(dir)
	if err != nil {
		return err
	}
	for _, d := range dirInfo {
		_, ok := filesSaveMap[d.Name()]
		if !ok {
			glog.Infof("remove %s", filepath.Join(dir, d.Name()))
			err = os.RemoveAll(filepath.Join(dir, d.Name()))
			if err != nil {
				return err
			}
		}
	}
	glog.Info("cleaned ", dir)
	return nil
}

func ResovleBinaryPath(binPath string) (string, error) {
	if filepath.IsAbs(binPath) {
		if _, err := os.Stat(binPath); err != nil {
			return binPath, fmt.Errorf("when resolving %s, os.Stat failed: %v", binPath, err)
		}
		return binPath, nil
	}
	// exec.LookPath will silently allow relative path, so we manually check it.
	if strings.Contains(binPath, string(filepath.Separator)) {
		// If invoked by checker_integration.cmd.main, the clang bin might be a relative path instead of executable in $PATH.
		absBinPath, err := filepath.Abs(binPath)
		if err != nil {
			return binPath, fmt.Errorf("when resolving %s, failed to convert to abs path: %v", binPath, err)
		}
		if _, err := os.Stat(absBinPath); err != nil {
			return absBinPath, fmt.Errorf("when resolving %s, os.Stat failed: %v", binPath, err)
		}
		return absBinPath, nil
	} else {
		if _, err := exec.LookPath(binPath); err != nil {
			return binPath, fmt.Errorf("when resolving %s, not found in $PATH: %v", binPath, err)
		}
		return binPath, nil // The binary is in $PATH, just leave it alone
	}
}
