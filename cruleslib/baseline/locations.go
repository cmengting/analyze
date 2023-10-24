/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package baseline

import pb "naive.systems/analyzer/analyzer/proto"

type Locations []*pb.ErrorLocation

func (l Locations) Len() int {
	return len(l)
}

func (l Locations) Less(i, j int) bool {
	if l[i].Path == l[j].Path {
		return l[i].LineNumber < l[j].LineNumber
	} else {
		return l[i].Path < l[j].Path
	}
}

func (l Locations) Swap(i, j int) {
	l[i], l[j] = l[j], l[i]
}
