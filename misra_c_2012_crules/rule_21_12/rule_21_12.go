/*
NaiveSystems Analyze - A tool for static code analysis
Copyright (C) 2023  Naive Systems Ltd.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

package rule_21_12

import (
	"strings"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
	"naive.systems/analyzer/cruleslib/options"
	"naive.systems/analyzer/cruleslib/runner"
	"naive.systems/analyzer/misra/checker_integration"
)

func Analyze(srcdir string, opts *options.CheckOptions) (*pb.ResultsList, error) {
	if opts.JsonOption.Standard == "c89" || opts.JsonOption.Standard == "c90" {
		return &pb.ResultsList{}, nil
	}
	ruleName := "misra_c_2012/rule_21_12"
	isMisra2023 := opts.JsonOption.Misra2023 != nil && *opts.JsonOption.Misra2023
	if isMisra2023 {
		ruleName = "misra_c_2012/rule_21_12_amd3"
	}
	results, err := runner.RunCppcheck(srcdir, ruleName, checker_integration.Cppcheck_STU, opts)
	if err != nil {
		glog.Error(err)
		return nil, err
	}
	for _, v := range results.Results {
		if strings.HasPrefix(v.ErrorMessage, "[C0409][misra-c2012-21.12]") {
			if isMisra2023 {
				v.ErrorMessage = "[C0409][misra-c2012-21.12]: 不得使用标准头文件 <fenv.h>"
				v.ErrorKind = pb.Result_MISRA_C_2012_RULE_21_12_AMD3
			} else {
				v.ErrorMessage = "[C0409][misra-c2012-21.12]: 不应使用 <fenv.h> 的异常处理特性"
				v.ErrorKind = pb.Result_MISRA_C_2012_RULE_21_12
			}
		}
	}
	return results, nil
}
