/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

package utils

func IntMax(a, b int) int {
	if a > b {
		return a
	} else {
		return b
	}
}

func IntMin(a, b int) int {
	if a < b {
		return a
	} else {
		return b
	}
}

type RecursiveTarjanSCCParams struct {
	dfn, low         map[string]int
	dfnCnt, stackCnt int
	visited          map[string]bool
	stack            []string
	result           [][]string
	graph            *map[string](map[string]struct{})
}

// Notice: Strongly Connected Components with only one node will *NOT* be reported by this function.
func RecursiveTarjanSCC(graph *map[string](map[string]struct{})) [][]string {
	result := make([][]string, 0)
	param := RecursiveTarjanSCCParams{}
	param.dfn = make(map[string]int)
	param.low = make(map[string]int)
	param.visited = make(map[string]bool)
	param.stack = make([]string, 0)
	param.result = make([][]string, 0)
	param.graph = graph
	for node := range *graph {
		_, ok := param.dfn[node]
		if !ok {
			RecursiveTarjanSCCSubroutine(node, &param)
			result = append(result, param.result...)
			param.result = make([][]string, 0)
		}
	}
	return result
}

func RecursiveTarjanSCCSubroutine(u string, p *RecursiveTarjanSCCParams) {
	p.visited[u] = true
	p.dfn[u] = p.dfnCnt
	p.low[u] = p.dfnCnt
	p.dfnCnt += 1
	p.stack = append(p.stack[:p.stackCnt], u)
	p.stackCnt += 1
	for v := range (*p.graph)[u] {
		_, ok := p.dfn[v]
		if !ok {
			RecursiveTarjanSCCSubroutine(v, p)
			p.low[u] = IntMin(p.low[u], p.low[v])
		} else if p.visited[v] {
			p.low[u] = IntMin(p.low[u], p.low[v])
		}
	}
	if p.dfn[u] == p.low[u] {
		chain := make([]string, 0)
		chain = append(chain, u)
		for {
			v := p.stack[p.stackCnt-1]
			p.stackCnt -= 1
			p.visited[v] = false
			if v == u {
				break
			}
			chain = append(chain, v)
		}
		if len(chain) > 1 {
			p.result = append(p.result, chain)
		}
	}
}
