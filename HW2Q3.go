package main

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
	"math/rand"
)

// Ticket Lock
type TicketLock struct {
	nextTicket uint32
	nowServing uint32
}
func (tl *TicketLock) Lock() {
	myTicket := atomic.AddUint32(&tl.nextTicket, 1) - 1
	for {
		if atomic.LoadUint32(&tl.nowServing) == myTicket {
			return
		}
	}
}
func (tl *TicketLock) Unlock() {
	atomic.AddUint32(&tl.nowServing, 1)
}
// CAS Spin Lock
type CASLock struct {
	state int32
}
func (l *CASLock) Lock() {
	for !atomic.CompareAndSwapInt32(&l.state, 0, 1) {
		// spin
	}
}
func (l *CASLock) Unlock() {
	atomic.StoreInt32(&l.state, 0)
}

// Benchmarking Function
func benchmark(lockType string, lock interface {
	Lock()
	Unlock()
}, numGoroutines int, iterations int) time.Duration {
	var wg sync.WaitGroup
	var totalWait int64
	work := func(id int) {
		defer wg.Done()
		r := rand.New(rand.NewSource(time.Now().UnixNano() + int64(id)))
		for i := 0; i < iterations; i++ {
			start := time.Now()
			lock.Lock()
			wait := time.Since(start).Microseconds()
			atomic.AddInt64(&totalWait, wait)
			time.Sleep(time.Duration(r.Intn(50)) * time.Microsecond)
			lock.Unlock()
		}
	}
	wg.Add(numGoroutines)
	for i := 0; i < numGoroutines; i++ {
		go work(i)
	}
	wg.Wait()
	avgWait := time.Duration(totalWait / int64(numGoroutines*iterations))
	return avgWait
}

// Main
func main() {
	iterations := 100
	threadCounts := []int{2, 4, 8, 16}
	fmt.Println("Lock Performance Comparison:")
	fmt.Println("---------------------------------------------------")
	for _, threads := range threadCounts {
		tlWait := benchmark("TicketLock", &TicketLock{}, threads, iterations)
		casWait := benchmark("CASLock", &CASLock{}, threads, iterations)
		fmt.Printf("%2d goroutines | TicketLock: %v µs | CASLock: %v µs\n",
			threads, tlWait, casWait)
	}
}
