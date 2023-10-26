/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package analyzer

import (
	"testing"

	"github.com/golang/glog"
	pb "naive.systems/analyzer/analyzer/proto"
)

var pathOne = "/src"
var pathTwo = "/tmp"
var lineNumOne = int32(5)
var lineNumTwo = int32(6)
var errMsgOne = "[C1702][misra-c2012-14.3]: Violation of 001"

var resultA = pb.Result{
	Path:          pathOne,
	LineNumber:    lineNumOne,
	ErrorMessage:  errMsgOne,
	FalsePositive: false,
}

var resultB = pb.Result{
	Path:          pathTwo,
	LineNumber:    lineNumOne,
	ErrorMessage:  errMsgOne,
	FalsePositive: false,
}

var resultC = pb.Result{
	Path:          pathOne,
	LineNumber:    lineNumTwo,
	ErrorMessage:  errMsgOne,
	FalsePositive: false,
}

var resultD = pb.Result{
	Path:          pathTwo,
	LineNumber:    lineNumTwo,
	ErrorMessage:  errMsgOne,
	FalsePositive: false,
}

var resultFalsePositiveD = pb.Result{
	Path:          pathTwo,
	LineNumber:    lineNumTwo,
	ErrorMessage:  errMsgOne,
	FalsePositive: true,
}

func TestDeleteFalsePositiveResults(t *testing.T) {
	var (
		in = pb.ResultsList{
			Results: []*pb.Result{
				&resultA,
				&resultB,
				&resultC,
				&resultD,
				&resultFalsePositiveD,
			},
		}
		expected = pb.ResultsList{
			Results: []*pb.Result{
				&resultA,
				&resultB,
				&resultC,
				// &resultD, D is removed by resultFalsePositiveI
				// &resultFalsePositiveD, is also removed for no noise
			},
		}
	)
	err := DeleteFalsePositiveResults(&in)
	if err != nil {
		glog.Error(err)
	}
	for i := 0; i < len(in.Results); i++ {
		if in.Results[i].Path != expected.Results[i].Path ||
			in.Results[i].LineNumber != expected.Results[i].LineNumber ||
			in.Results[i].ErrorMessage != expected.Results[i].ErrorMessage {
			t.Errorf("Wrong Result in position %d", i)
		}
	}
}
