package main

import (
	"fmt"
	"io/fs"
	"os"
	"strings"
	"sync"
	"time"
)

func main() {
	paths := os.Getenv("PATH")
	pathList := strings.Split(paths, ":")
	fmt.Println(pathList)

	binCount := 0
	start := time.Now()
	var wg sync.WaitGroup
	for _, path := range pathList {
		wg.Add(1)
		go func() {
			defer wg.Done()
			NoobApproach(path)
		}()
	}

	wg.Wait()

	// No concurrency best  = 1.7
	// Even with Goroutines = 1.7
	// Probably next, Pools?
	end := time.Now()
	fmt.Println(binCount, end.Sub(start))
}

func NoobApproach(path string) int {
	entries, err := os.ReadDir(path)
	if err != nil {
		return 0
	}

	binCount := 0
	for _, entry := range entries {
		if entry.Type() != fs.ModeDir {
			binCount++
		}
	}
	return binCount
}
