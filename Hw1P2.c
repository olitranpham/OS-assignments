// HW1 part 2 - Threaded version
// Group: Cody Nguyen, Ian Khoo, Olivia Pham

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define LIMIT 10000 // increased the workload

// Timing helper
double elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

// Shared data
int num;
int ready = 0; // flag to signal data is ready

// Synchronization primitives
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consume = PTHREAD_COND_INITIALIZER;

// Consumer thread
void* consumer(void* arg) {
    for (int i = 0; i < LIMIT; i++) {
        pthread_mutex_lock(&lock);

        // Wait until there's data to consume
        while (!ready) {
            pthread_cond_wait(&cond_consume, &lock);
        }

        printf("Consumer: %d\n", num);
        fflush(stdout);

        // Mark as consumed
        ready = 0;

        // Signal producer that data was consumed
        pthread_cond_signal(&cond_produce);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void run_threads() {
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    for (int i = 1; i <= LIMIT; i++) {
        pthread_mutex_lock(&lock);
        while (ready) {
            pthread_cond_wait(&cond_produce, &lock);
        }
        num = i;
        // printf("Producer: %d\n", num);
        ready = 1;
        pthread_cond_signal(&cond_consume);
        pthread_mutex_unlock(&lock);
    }

    pthread_join(consumer_thread, NULL);
}

void run_processes() {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();

    if (pid == 0) {  // Consumer
        close(fd[1]);
        int val;
        for (int i = 0; i < LIMIT; i++) {
            read(fd[0], &val, sizeof(val));
            // printf("Consumer: %d\n", val);
        }
        close(fd[0]);
        exit(0);
    } else {         // Producer
        close(fd[0]);
        for (int i = 1; i <= LIMIT; i++) {
            write(fd[1], &i, sizeof(i));
            // printf("Producer: %d\n", i);
        }
        close(fd[1]);
        wait(NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [--threads | --process]\n", argv[0]);
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (strcmp(argv[1], "--threads") == 0) {
        run_threads();
    } else if (strcmp(argv[1], "--process") == 0) {
        run_processes();
    } else {
        printf("Invalid option.\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Elapsed time (%s): %.6f seconds\n", argv[1], elapsed(start, end));

    return 0;
}