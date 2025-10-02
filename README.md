# Hw1-Part2

- One-producer, one-consumer problem implemented two ways:
-   Processes (using fork() and pipes for communication)
-   Threads (using pthreads, mutex, and condition variables for synchronization)
-   Producer generates numbers 1–10,000, Consumer receives them
-   Execution time is measured to compare performance of process-based vs. thread-based approaches

  # Run Instructions
  - gcc Hw1P2.c -o hw1p2 -lpthread
- ./hw1p2 --threads     # Run thread-based version
- ./hw1p2 --process     # Run process-based version
