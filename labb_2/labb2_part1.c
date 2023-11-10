#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_BUFFER 15

// TODO, make the repetetive stuff work in a loop depending on
// the amount of threads we want to run, so maybe add an
// #define THREADS 3
// or something

// Global variables
int buffer = 0;
pthread_mutex_t mutex;

// Thread function
void *thread_function(void *arg) {
    int increments = 0;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (buffer >= MAX_BUFFER) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Print thread ID, process ID, and buffer value
        printf("TID: %lu, PID: %d, Buffer: %d\n", pthread_self(), getpid(), buffer);

        // Increment buffer and keep track of increments
        buffer++;
        increments++;

        pthread_mutex_unlock(&mutex);
        
        // Small delay for demonstration purposes
        usleep(100000);
    }

    pthread_exit((void *)(intptr_t)(increments));
}

int main() {
    pthread_t thread1, thread2, thread3;
    int totalIncrements = 0;
    void *ret;

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Create threads
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    pthread_create(&thread3, NULL, thread_function, NULL);

    // Join threads and get return value
    pthread_join(thread1, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread1, (int)(intptr_t)ret);
    totalIncrements += (int)(intptr_t)ret;

    pthread_join(thread2, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread2, (int)(intptr_t)ret);
    totalIncrements += (int)(intptr_t)ret;

    pthread_join(thread3, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread3, (int)(intptr_t)ret);
    totalIncrements += (int)(intptr_t)ret;

    printf("Total buffer accesses: %d\n", totalIncrements);

    // Clean up
    pthread_mutex_destroy(&mutex);

    return 0;
}
