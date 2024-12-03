package main

import (
	"context"
	"fmt"
	"log"
	"net/http"
	_ "net/http/pprof"
	"runtime/pprof"
	"sync"
	"time"
)

func oneWorkItem(_ context.Context, leakySliceEntry []string) {
	fmt.Printf("Start: %v\n", time.Now())

	// Memory
	for i := 0; i < 500000; i++ {
		leakySliceEntry = append(leakySliceEntry, "aaaa")
	}

	// Blocking
	time.Sleep(2 * time.Second)
	fmt.Printf("End: %v\n", time.Now())
}

// Some function that does work
func hardWork(ctx context.Context, wg *sync.WaitGroup, leakySlice [][]string) {
	defer wg.Done()

	var counter int
	for {
		pprof.Do(ctx,
			pprof.Labels("instance", fmt.Sprintf("instance%d", counter)),
			func(ctx context.Context) {
				leakySliceEntry := make([]string, 0)
				leakySlice = append(leakySlice, leakySliceEntry)
				oneWorkItem(ctx, leakySliceEntry)
			})
		counter++
	}
}

func main() {
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		fmt.Println("Listening on localhost:6060 for pprof, use \"go tool pprof -seconds 5 http://localhost:6060/debug/pprof/heap\" to check heap profile.")
		log.Printf("Listener started: %s\n", http.ListenAndServe("localhost:6060", nil))
		fmt.Println("Done listening")
		wg.Done()
	}()
	wg.Add(1)
	ctx := pprof.WithLabels(context.Background(), pprof.Labels("name", "hardWork"))
	var leakySlice [][]string
	go hardWork(ctx, &wg, leakySlice)
	wg.Wait()
}
