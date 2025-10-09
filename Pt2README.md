# Design:
We implemented:
- Baseline (global lock) - One mutex guards the entire list. All insert and lookup operations acquire this lock, which keeps the list safe from concurrent access but serializes all operations. This is simple and has low locking overhead, but does not scale well with many threads
- Hand-over-Hand (HOH) - Each node has its own lock. Threads acquire locks on nodes as they traverse the list, releasing the previous node after locking the next. This allows multiple threads to safely access different parts of the list at the same time, which can improve scalability in read-heavy workloads.
- Benchmark design - Each thread performs 100,000 operations with 80% lookups and 20% inserts. We chose this mix to simulate a realistic workload where reads are more common than writes. We measure total elapsed time and compute throughput in operations per second.
  
# Analysis of benchmark results:
In our benchmark, the baseline version with a single global lock was faster than the hand-over-hand fine-grained locking version. Even though HOH is supposed to reduce contention and let threads work more independently, the extra time spent locking and unlocking each node made it slower for this workload. Most operations were at the head of the list, so threads were still competing for the same part of the list. This shows that fine-grained locking works best when threads are accessing different parts of a list, while global locking can be faster for smaller lists or when operations are focused at the head. This relates to Go's design philosophy of sharing memory by communicating because it highlights that careful management of shared memory, or avoiding it when possible, can improve performance and reduce contention.

# Run Instructions
gcc -pthread -O2 concurrent_list.c -o concurrent_list

// run baseline
./concurrent_list baseline

// run hand-over-hand
./concurrent_list hoh
