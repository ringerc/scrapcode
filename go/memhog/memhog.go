/*
 * If you're reading this wretched code, I'm sorry
 */
package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"runtime"
	"runtime/debug"
	"sync"
	"syscall"
	"time"

	"github.com/c2h5oh/datasize"
)

type Config struct {
	sizeBytes              uint64
	nThreads               uint64
	sleepSecondsBeforeFree int
	sleepSecondsBeforeExit int
	statsIntervalSeconds   int
}

func printMemStats() {
	var m runtime.MemStats
	runtime.ReadMemStats(&m)
	fmt.Printf("\n")
	fmt.Printf("%50s: %12d bytes\n", "Currently allocated heap memory (HeapAlloc)", m.HeapAlloc)
	fmt.Printf("%50s: %12d bytes\n", "Idle heap memory (HeapAlloc)", m.HeapIdle)
	fmt.Printf("%50s: %12d bytes\n", "Runtime memory released to OS (HeapReleased)", m.HeapReleased)
	fmt.Printf("%50s: %12d bytes\n", "Idle non-released memory (HeapAlloc-HeapReleased)", m.HeapIdle-m.HeapReleased)
	fmt.Printf("%50s: %12d bytes\n", "Cumulative allocated memory (TotalAlloc)", m.TotalAlloc)
	fmt.Printf("%50s: %12d bytes\n", "Runtime memory allocated high-water mark (Sys)", m.Sys)
	fmt.Printf("%50s: %12d bytes\n", "Runtime memory still allocated (Sys-HeapReleased)", m.Sys-m.HeapReleased)
}

func main() {
	var size string
	var nThreads int
	var config Config
	flag.StringVar(&size, "size", "1G", "Size in bytes with optional SI suffix")
	flag.IntVar(&nThreads, "threads", 10, "number of writer threads")
	flag.IntVar(&config.sleepSecondsBeforeFree, "sleep-seconds-before-free", -1, "Number of seconds to sleep after allocation before attempting to release memory. -1 means never release, 0 means no sleep.")
	flag.IntVar(&config.sleepSecondsBeforeExit, "sleep-seconds-before-exit", -1, "Number of seconds to sleep before exiting. -1 means never exit, 0 means exit immediately.")
	flag.IntVar(&config.statsIntervalSeconds, "stats-interval-seconds", -1, "Seconds to sleep between emitting memory stats during pre-exit sleep, or -1 to only print once")
	flag.Parse()

	var parsedSize datasize.ByteSize
	err := parsedSize.UnmarshalText([]byte(size))
	if err != nil {
		log.Fatalf("parsing size failed: %v", err)
	}
	config.sizeBytes = parsedSize.Bytes()

	if nThreads <= 0 {
		log.Fatalf("Invalid nthreads: %d", nThreads)
	}
	config.nThreads = uint64(nThreads)

	// We only need to write into each page, not fill each page, unless
	// there is some kind of memory compression in use. This makes filling
	// lots of memory faster.
	fmt.Printf("Detecting platform page size... ")
	pageSize := uint64(syscall.Getpagesize())
	fmt.Printf("%v bytes\n", pageSize)

	fmt.Printf("Attempting to allocate %s...\n", parsedSize.HumanReadable())

	data := make([][]byte, config.nThreads)
	threadBytes := config.sizeBytes / config.nThreads
	var wg sync.WaitGroup

	writerFunc := func(threadno int) {
		data[threadno] = make([]byte, threadBytes)
		for i := uint64(0); i < threadBytes; i += pageSize {
			data[threadno][i] = byte(i % 256)
		}
		wg.Done()
	}
	wg.Add(int(config.nThreads))
	for n := range config.nThreads {
		go writerFunc(int(n))
	}
	wg.Wait()

	fmt.Println("Memory allocated and filled.")
	printMemStats()

	// Print memory statistics to observe the heap size

	if config.sleepSecondsBeforeFree >= 0 {
		fmt.Printf("\nSleeping %v seconds before freeing memory...", config.sleepSecondsBeforeFree)
		time.Sleep(time.Duration(int64(config.sleepSecondsBeforeFree) * int64(time.Second)))
		fmt.Printf(" done\n")

		fmt.Printf("\nTrying to release memory...")
		data = make([][]byte, 0)
		// Force aggressive GC
		debug.SetMemoryLimit(0)
		runtime.GC()
		// This might take time
		debug.FreeOSMemory()
		fmt.Printf(" done\n\n")

		printMemStats()
	}

	// Because k8s doesn't have swap, it can't page out our unused memory
	// to relieve memory pressure. It's safe to allocate and fill the memory
	// then sleep indefinitely, there is no need to continually read it or
	// write to it like there would be on a sensible Linux system with swap.

	sleepContext := context.Background()
	if config.sleepSecondsBeforeExit >= 0 {
		sleepContext, _ = context.WithTimeout(sleepContext, time.Duration(int64(config.sleepSecondsBeforeExit)*int64(time.Second)))
	}

	var statsTicker *time.Ticker
	if config.statsIntervalSeconds > 0 {
		statsTicker = time.NewTicker(time.Duration(int64(config.statsIntervalSeconds) * int64(time.Second)))
	} else {
		// Fake ticker that never fires, because waiting on a nil timer
		// is not legal
		statsTicker = &time.Ticker{
			C: make(chan time.Time),
		}
	}

	// To stop go complaining about a deadlock when we intentionally sleep forever
	// if there's no exit timer, explicitly wait for SIGINT
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	for {
		select {
		case <-statsTicker.C:
			printMemStats()
		case <-sleepContext.Done():
			fmt.Printf("Deadline expired\n")
			os.Exit(0)
		case sig := <-sigs:
			fmt.Printf("Exiting: %v\n", sig)
			os.Exit(0)
		}
	}

	if config.sleepSecondsBeforeExit >= 0 {
		fmt.Printf("\nSleeping %v seconds before exit...", config.sleepSecondsBeforeExit)
		time.Sleep(1)
		fmt.Printf(" done\n")
	} else {
		fmt.Printf("\nSleeping until ^C...\n")
		for {
			time.Sleep(1 * time.Second)
			printMemStats()
		}
	}
}
