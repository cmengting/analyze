/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package atomic

import (
	"fmt"
	"os"
	"path/filepath"
)

func Write(name string, data []byte) error {
	pattern := "tmp-*-" + filepath.Base(name)
	f, err := os.CreateTemp(filepath.Dir(name), pattern)
	if err != nil {
		return fmt.Errorf("os.CreateTemp: %v", err)
	}
	defer os.Remove(f.Name())
	// Explicitly set the permissions of the temporary file to 0644
	if err := os.Chmod(f.Name(), 0644); err != nil {
		return fmt.Errorf("os.Chmod: %v", err)
	}
	_, err = f.Write(data)
	if err != nil {
		return fmt.Errorf("failed to write to file %s: %v", f.Name(), err)
	}
	err = os.Rename(f.Name(), name)
	if err != nil {
		return fmt.Errorf("failed to rename file %s to %s: %v", f.Name(), name, err)
	}
	return nil
}
