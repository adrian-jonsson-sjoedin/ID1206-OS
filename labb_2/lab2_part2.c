#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#define MAX 5

// Shared structure
struct shared_memory {
    int VAR;
    sem_t readLock;
    sem_t writeLock;
    int readCount;
};

int main() {
    // Initialize shared memory
    struct shared_memory *shared = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared->VAR = 0;
    shared->readCount = 0;

    // Initialize semaphores
    sem_init(&shared->readLock, 1, 1);
    sem_init(&shared->writeLock, 1, 1);

    // Create one writer and two reader processes
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            if (i == 0) {  // Writer process
                while (shared->VAR < MAX) {
                    sem_wait(&shared->writeLock);  // Wait for no readers
                    printf("The writer acquires the lock.\n");
                    shared->VAR++;
                    printf("The writer (%d) writes the value %d\n", getpid(), shared->VAR);
                    printf("The writer releases the lock.\n");
                    sem_post(&shared->writeLock);
                    sleep(1);
                }
            } else {  // Reader processes
                while (shared->VAR < MAX) {
                    sem_wait(&shared->readLock);
                    shared->readCount++;
                    if (shared->readCount == 1) {
                        sem_wait(&shared->writeLock);  // First reader blocks writers
                        printf("The first reader acquires the lock.\n");
                    }
                    sem_post(&shared->readLock);

                    // Reading
                    printf("The reader (%d) reads the value %d\n", getpid(), shared->VAR);

                    sem_wait(&shared->readLock);
                    shared->readCount--;
                    if (shared->readCount == 0) {
                        printf("The last reader releases the lock.\n");
                        sem_post(&shared->writeLock);  // Last reader unblocks writers
                    }
                    sem_post(&shared->readLock);
                    sleep(1);
                }
            }
            exit(0);  // Child process exits
        }
    }

    // Parent process waits for all child processes
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    // Clean up
    sem_destroy(&shared->readLock);
    sem_destroy(&shared->writeLock);
    munmap(shared, sizeof(struct shared_memory));

    return 0;
}
