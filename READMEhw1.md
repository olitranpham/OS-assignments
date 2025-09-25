* Hw1-Part2

Implements two approaches for a producer–consumer system with a workload of 10,000 items.

Threaded Version (--threads)

Uses a producer thread to generate numbers and a consumer thread to process them.

Synchronization is handled with mutex locks and condition variables to alternate production/consumption.

Process Version (--process)

Uses fork() to create a child process (consumer) and a parent process (producer).

Communication is done via a pipe (write by producer, read by consumer).

Both versions measure and print the elapsed execution time for performance comparison.
