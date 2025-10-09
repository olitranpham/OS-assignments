// HW3 Group: Ian Khoo, Olivia Pham, Cody Nguyen
// Code adapted from: 
// "The Art of Multiprocessor Programming" (Herlihy & Shavit, 2012)
// AI usage: ChatGPT helped suggest benchmark structure and timing helper implementation

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#define NUM_THREADS 8
#define OPS_PER_THREAD 100000
#define KEY_RANGE 1000

// node structure for linked list
typedef struct node_t {
    int key;                  // key stored in node
    struct node_t *next;      // pointer to next node
    pthread_mutex_t lock;     // used for hand-over-hand fine-grained locking
} node_t;

// list structure
typedef struct list_t {
    node_t *head;             // head of the linked list
    pthread_mutex_t lock;     // global lock for baseline version
} list_t;

// ========== baseline linked list (global lock) ==========

// initialize linked list
void list_init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL); // initialize global lock
}

// insert at the head of the list (with global lock)
int list_insert(list_t *L, int key) {
    node_t *newnode = malloc(sizeof(node_t));
    if (newnode == NULL) return -1;

    newnode->key = key;

    pthread_mutex_lock(&L->lock);      // lock entire list
    newnode->next = L->head;
    L->head = newnode;
    pthread_mutex_unlock(&L->lock);    // release global lock

    return 0;
}

// lookup a key in the list using global lock
int list_lookup(list_t *L, int key) {
    int found = -1;
    pthread_mutex_lock(&L->lock);

    node_t *curr = L->head;
    while (curr != NULL) {
        if (curr->key == key) {
            found = 0;                  // key found
            break;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&L->lock);
    return found;
}

// ========== hand-over-hand linked list (fine-grained lock) ==========

// initialize list for hand-over-hand locking
void hoh_list_init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

// insert at the head using hand-over-hand locking
int hoh_list_insert(list_t *L, int key) {
    node_t *newnode = malloc(sizeof(node_t));
    if (newnode == NULL) return -1;

    newnode->key = key;
    pthread_mutex_init(&newnode->lock, NULL); // initialize node lock

    pthread_mutex_lock(&L->lock);             // lock head pointer
    newnode->next = L->head;
    if (L->head != NULL) pthread_mutex_lock(&L->head->lock); // lock old head
    L->head = newnode;
    pthread_mutex_unlock(&L->lock);           // release head lock

    if (newnode->next != NULL) pthread_mutex_unlock(&newnode->next->lock); // release old head lock
    return 0;
}

// lookup using hand-over-hand locking
int hoh_list_lookup(list_t *L, int key) {
    int found = -1;

    pthread_mutex_lock(&L->lock);             // lock list head
    node_t *curr = L->head;
    if (curr != NULL) pthread_mutex_lock(&curr->lock); // lock first node
    pthread_mutex_unlock(&L->lock);           // release list head lock

    while (curr) {
        if (curr->key == key) {
            found = 0;
            pthread_mutex_unlock(&curr->lock);
            return found;
        }

        node_t *next = curr->next;
        if (next != NULL) pthread_mutex_lock(&next->lock); // lock next node before moving
        pthread_mutex_unlock(&curr->lock);                 // unlock previous node
        curr = next;
    }

    return found;
}

// ========== benchmark worker ==========

typedef struct {
    list_t *list;
    int use_hoh;  // flag: 0 = baseline, 1 = hand-over-hand
    int id;       // thread id, used to seed random generator
} thread_arg_t;

// each thread repeatedly performs lookups/inserts
void *worker(void *arg) {
    thread_arg_t *t = (thread_arg_t *) arg;
    unsigned int seed = t->id * 12345 + 1; // unique seed per thread

    for (int i = 0; i < OPS_PER_THREAD; i++) {
        int op = rand_r(&seed) % 100;
        int key = rand_r(&seed) % KEY_RANGE;

        if (op < 80) { // 80% lookups
            if (t->use_hoh)
                hoh_list_lookup(t->list, key);
            else
                list_lookup(t->list, key);
        } else { // 20% inserts
            if (t->use_hoh)
                hoh_list_insert(t->list, key);
            else
                list_insert(t->list, key);
        }
    }
    return NULL;
}

// ========== timing helper ========== 

// returns current time in milliseconds
double now_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

// ========== benchmark runner ==========

// runs benchmark for baseline or HOH list
void run_benchmark(int use_hoh) {
    list_t list;
    if (use_hoh)
        hoh_list_init(&list);
    else
        list_init(&list);

    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];

    double start = now_ms();  // start timing

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].list = &list;
        args[i].use_hoh = use_hoh;
        args[i].id = i;
        pthread_create(&threads[i], NULL, worker, &args[i]); // spawn thread
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL); // wait for threads

    double end = now_ms();            // end timing
    double elapsed = end - start;

    printf("\n--- results ---\n");
    printf("threads: %d\n", NUM_THREADS);
    printf("ops per thread: %d\n", OPS_PER_THREAD);
    printf("total ops: %d\n", NUM_THREADS * OPS_PER_THREAD);
    printf("time: %.2f ms\n", elapsed);
    printf("throughput: %.2f ops/sec\n",
           (NUM_THREADS * OPS_PER_THREAD) / (elapsed / 1000.0)); // compute ops/sec
}

// ========== main ==========

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s [baseline|hoh]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "baseline") == 0) {
        printf("running baseline version (global lock)...\n");
        run_benchmark(0);
    } else if (strcmp(argv[1], "hoh") == 0) {
        printf("running hand-over-hand version...\n");
        run_benchmark(1);
    } else {
        printf("unknown option: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
