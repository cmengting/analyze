/*
Copyright 2023 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package cpplint

import (
	"os/exec"
	"regexp"
	"strconv"

	"github.com/golang/glog"
	"naive.systems/analyzer/cruleslib/basic"
)

type Report struct {
	Path       string
	LineNumber int
	Message    string
	Category   string
	Confidence int
}

func ParseReports(out []byte, srcdir string) ([]Report, error) {
	reports := []Report{}
	re := regexp.MustCompile(`(.*):(\d*):  (.*)  \[(.*)\] \[(\d)\]`)
	matches := re.FindAllStringSubmatch(string(out), -1)
	for _, match := range matches {
		linenum, err := strconv.Atoi(match[2])
		if err != nil {
			glog.Errorf("cpplint output cannot be parsed: %s", string(out))
			return reports, err
		}
		confidence, err := strconv.Atoi(match[5])
		if err != nil {
			glog.Errorf("cpplint output cannot be parsed: %s", string(out))
			return reports, err
		}
		filename := match[1]
		path, err := basic.ConvertRelativePathToAbsolute(srcdir, filename)
		if err != nil {
			glog.Error(err)
			continue
		}
		reports = append(reports, Report{
			Path:       path,
			LineNumber: linenum,
			Message:    match[3],
			Category:   match[4],
			Confidence: confidence,
		})
	}
	return reports, nil
}

type Runner struct {
	PythonBin     string
	Script        string
	TaskName      string
	LimitMemory   bool
	TimeoutNormal int
	TimeoutOom    int
	Srcdir        string
}

func (r *Runner) Execute(cmd *exec.Cmd) (reports []Report, err error) {
	glog.Info("executing: ", cmd.String())
	out, err := basic.CombinedOutput(cmd, r.TaskName, r.LimitMemory, r.TimeoutNormal, r.TimeoutOom)
	// err == nil ==> no report
	if err == nil {
		return
	}
	if serr, ok := err.(*exec.ExitError); !ok || serr.ExitCode() != 1 {
		glog.Errorf("cpplint execution error: executing %s, reported:\n%s", cmd.String(), string(out))
		return
	}
	// ExitError 1 ==> normal report
	reports, err = ParseReports(out, r.Srcdir)
	if err != nil {
		glog.Errorf("parse cpplint output error: %v", err)
	}
	return
}

func (r *Runner) Run(srcdir, filter string) (reports []Report, err error) {
	return r.Execute(exec.Command(r.PythonBin, r.Script, "--filter="+filter, "--recursive", srcdir))
}

func (r *Runner) RunRoot(srcdir, rootdir, filter string) (reports []Report, err error) {
	return r.Execute(exec.Command(r.PythonBin, r.Script, "--filter="+filter, "--recursive", "--root="+rootdir, srcdir))
}
