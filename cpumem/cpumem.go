/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package cpumem

import (
	"fmt"
	"sync"
	"time"

	"github.com/golang/glog"
	"naive.systems/analyzer/cruleslib/basic"
)

var (
	remainLock sync.Mutex
	remainCond *sync.Cond
	remainCpu  int
	remainMem  int
	totalCpu   int
	totalMem   int
)

func Init(cpu, mem int) {
	remainCond = sync.NewCond(&remainLock)
	remainCpu = cpu
	remainMem = mem
	totalCpu = cpu
	totalMem = mem
}

func Acquire(cpu, mem int, taskName string) error {
	cpuExceedMessage := ""
	memExceedMessage := ""
	if cpu > totalCpu {
		cpuExceedMessage = fmt.Sprintf("%s aquired %d cpus, but total %d cpus available\n", taskName, cpu, totalCpu)
	}
	if mem > totalMem {
		memExceedMessage = fmt.Sprintf("%s aquired %d KB memory, but total %d KB memory available\n", taskName, cpu, totalCpu)
	}
	if cpuExceedMessage+memExceedMessage != "" {
		return fmt.Errorf(cpuExceedMessage + memExceedMessage)
	}
	start := time.Now()
	remainLock.Lock()
	for remainCpu < cpu || remainMem < mem {
		remainCond.Wait()
	}
	remainCpu -= cpu
	remainMem -= mem
	remainLock.Unlock()
	elapsed := time.Since(start)
	glog.Infof("%s waited for [%s] to acquire resources", taskName, basic.FormatTimeDuration(elapsed))
	remainCond.Signal()
	return nil
}

func Release(cpu, mem int) {
	remainLock.Lock()
	remainCpu += cpu
	remainMem += mem
	remainLock.Unlock()
	remainCond.Signal()
}

func GetTotalMem() int {
	return totalMem
}
