/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package compilecommand

import (
	"encoding/json"
	"io"
	"os"

	"github.com/golang/glog"
)

type CompileCommand struct {
	Command   string   `json:"command,omitempty"`
	Arguments []string `json:"arguments,omitempty"`
	File      string   `json:"file"`
	Directory string   `json:"directory"`
	Output    string   `json:"output,omitempty"`
}

const CC1 string = "-cc1"

func (cc CompileCommand) ContainsCC1() bool {
	for _, v := range cc.Arguments {
		if v == CC1 {
			return true
		}
	}
	return false
}

func ReadCompileCommandsFromFile(compileCommandsPath string) (*[]CompileCommand, error) {
	ccFile, err := os.Open(compileCommandsPath)
	if err != nil {
		glog.Error(err)
		return nil, err
	}

	defer ccFile.Close()

	byteContent, err := io.ReadAll(ccFile)
	if err != nil {
		return nil, err
	}

	commands := []CompileCommand{}
	err = json.Unmarshal(byteContent, &commands)
	if err != nil {
		return nil, err
	}

	return &commands, nil
}
