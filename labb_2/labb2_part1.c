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
/*
pthread_mutex_t is a data type representing a mutex in the POSIX threads (pthread) library, 
which is commonly used for multithreading programming in Unix-like operating systems
A mutex is a synchronization primitive used to protect shared resources, making it so that
only one thread can access the protected resources at a time.
*/
// pthread_mutex_t mutex;

// Constant that allows us to initialize the mutex with default attributes instead of using 
// pthread_mutex_init with null. Since passing null to it is the same as initializing with 
// default attributes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 


// Thread function
void *thread_function(void *arg) {
    // to keep track of how many times this thread increments the buffer
    int increments = 0;
    //infintie loop until buffer reaches desired size.
    while (1) {
        //lock the thread to ensure only one thread at a time can do the following code.
        //it is like synchronize in Java
        pthread_mutex_lock(&mutex);
        //release the lock and break out of the while loop
        if (buffer >= MAX_BUFFER) {
            pthread_mutex_unlock(&mutex); 
            break;
        }

        // Print thread ID, process ID, and buffer value
        printf("TID: %lu, PID: %d, Buffer: %d\n", pthread_self(), getpid(), buffer);

        // Increment buffer 
        buffer++;
        // Increment the thread tracker value
        increments++;
        // Unlock the thread to allow another thread to enter the critical section
        pthread_mutex_unlock(&mutex);
        
        // Small delay for demonstration purposes
        // no delay means only one thread is working, delay of 1 mixes things up, delay of 
        // 100 or more ensures equal distribution
        usleep(1);
    }
    //Terminates the threads and returns that threads increments value as the exit status
    pthread_exit((void *)(intptr_t)(increments));
    /*
    the intptr_t, is an integer type guaranteed to be able to hold a pointer. 
    It's often used for casting pointers to integers and vice versa. 
    The intptr_t cast allows you to pass an integer value as a pointer, and then you can 
    later retrieve this integer value when you join the thread.
    */
}

int main() {
    // Will be used to store the threads IDs
    pthread_t thread1, thread2, thread3;
    int totalThreadIncrements = 0;
    // To store the exit status returned by each thread when they are joined
    void *ret;

    // Initialize mutex
    // pthread_mutex_init(&mutex, NULL); //Using constant to initialize instead

    // Creating the threads and associates it with the function thread_function 
    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);
    pthread_create(&thread3, NULL, thread_function, NULL);

    // Join threads and get return value
    // Waits for thread1 to finish and retrieves the exit status from it, storing it in ret
    pthread_join(thread1, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread1, (int)(intptr_t)ret);
    //  thread1's number of increments are added to totalThreadIncrements
    totalThreadIncrements += (int)(intptr_t)ret;

    pthread_join(thread2, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread2, (int)(intptr_t)ret);
    totalThreadIncrements += (int)(intptr_t)ret;

    pthread_join(thread3, &ret);
    printf("TID %lu worked on the buffer %d times\n", thread3, (int)(intptr_t)ret);
    totalThreadIncrements += (int)(intptr_t)ret;

    printf("Total buffer accesses: %d\n", totalThreadIncrements);

    // Clean up by destroying the mutex
    // Might not be needed since the mutex was statically initialized with PTHREAD_MUTEX_INITIALIZER
    // but still good practice to do. 
    pthread_mutex_destroy(&mutex);

    return 0;
}
