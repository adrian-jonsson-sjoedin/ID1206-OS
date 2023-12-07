#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <math.h>



#define DISK_SIZE 5000
#define AMOUNT_OF_REQUESTS 1000

/* Function prototypes */
int FCFS(int *requests, int initial_position);
int SSTF(int *requests, int *requests_sorted, int initial_position);
int SCAN(int *requests, int *requests_sorted, int initial_position);
int CSCAN(int *requests, int *requests_sorted, int initial_position);
int Look(int *requests, int *requests_sorted, int initial_position);
int CLook(int *requests, int *requests_sorted, int initial_position);
int parseCmdArg(char *arg);
int comparator(const void * p1, const void * p2);
int findStartingIndex(int *array, int value);
void printArray(int *array, int size); 
int shortestDistance(int low, int mid, int high);

/* Simulated disk */
int disk[DISK_SIZE];

/* Array that holds the specified amount of requests*/
int requests[AMOUNT_OF_REQUESTS];

/* The request sorted in ascending order */
int requests_sorted[AMOUNT_OF_REQUESTS];

/*
Controlls the main program flow. Assumes that the argument is a valid integer number.
*/
int main(int argc, char *argv[]) {
    int takenIntegers[DISK_SIZE]; // Used for control when generating random numbers

    // Attempt with parsing the argument to set the initial head position.
    int initial_position;
    if (argc > 2) {
        printf("Error: Too many arguments!\n");
        exit(EXIT_FAILURE);              
    }
    else if (argc == 2) {
        initial_position = parseCmdArg(argv[1]);
        printf("Initial disk head position is set to: %d\n", initial_position);
    }
    else {
        printf("Error: Missing argument!\n");
        exit(EXIT_FAILURE);
    }

    // Populate the disk with the actual index values. This makes for easier testing.
    // Also use it to set the control array.
    for (int i = 0; i < DISK_SIZE; i++) {
        disk[i] = i;
        takenIntegers[i] = 0;
    }  

    // Populate the requests array with unique random requests ranging from 0-4999.
    srand(time(NULL)); 
    int counter = 0;
    while (counter < AMOUNT_OF_REQUESTS) {
        int random_number = rand() % DISK_SIZE; // Random number between 0 and 4999
        if (takenIntegers[random_number] == 0) {
            requests[counter] = random_number;
            takenIntegers[random_number] = 1; // Mark the number as taken
            counter++;
        }
        // Else the number was already taken
    }

    // Duplicate the requests and sort them since this will be used in some algorithms
    memcpy(requests_sorted, requests, AMOUNT_OF_REQUESTS * sizeof(int));
    qsort(requests_sorted, AMOUNT_OF_REQUESTS, sizeof(*requests), comparator);

    // Call the algorithms then display the result.
    printf("\nUsing FCFS\nTotal head movement: %d\n", FCFS(requests, initial_position));
    printf("\nUsing SSTF\nTotal head movement: %d\n", SSTF(requests, requests_sorted, initial_position));
    printf("\nUsing SCAN\nTotal head movement: %d\n", SCAN(requests, requests_sorted, initial_position));
    printf("\nUsing C-SCAN\nTotal head movement: %d\n", CSCAN(requests, requests_sorted, initial_position));
    printf("\nUsing Look\nTotal head movement: %d\n", Look(requests, requests_sorted, initial_position));
    printf("\nUsing C-Look\nTotal head movement: %d\n\n", CLook(requests, requests_sorted, initial_position));

    /* Debug code. */
    //printArray(requests, AMOUNT_OF_REQUESTS);
    //printArray(requests_sorted, AMOUNT_OF_REQUESTS);

    return 0;
}

/*
First come, first served. Serves the requests according to how they arrive.
*/
int FCFS(int *requests, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    for (int i = 0; i < AMOUNT_OF_REQUESTS; i++){
        total_movement += abs(current_position - requests[i]);
        current_position = requests[i];
    }
    return total_movement;
}

/*
Shortest seek time first. 
*/
int SSTF(int *requests, int *requests_sorted, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    int higher_index = findStartingIndex(requests_sorted, initial_position);
    int lower_index = higher_index - 1;
    for (int i = 0; i < AMOUNT_OF_REQUESTS; i++) {
        int shortest = shortestDistance(requests_sorted[lower_index], current_position, requests_sorted[higher_index]);
        if (shortest == requests_sorted[higher_index]) {
            total_movement += abs(current_position - requests_sorted[higher_index]);
            current_position = requests_sorted[higher_index];
            if (higher_index <= DISK_SIZE-1) {
                higher_index++;
            }    
        }
        else if (shortest == requests_sorted[lower_index]) {
            total_movement += abs(current_position - requests_sorted[lower_index]);
            current_position = requests_sorted[lower_index];
            if (lower_index >= 0) {
                lower_index--;
            }
        }
    }
    return total_movement;
}

/*
Moves the head from one end of the disk to the other like a zig-zag pattern.
*/
int SCAN(int *requests, int *requests_sorted, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    int starting_index = findStartingIndex(requests_sorted, initial_position);
    // Move left
    for (int i = starting_index-1; i >= 0; i--){
        total_movement += abs(current_position - requests_sorted[i]);
        current_position = requests_sorted[i];
    }
    total_movement += abs(DISK_SIZE - current_position);
    // Move right
    for (int j = starting_index; j < AMOUNT_OF_REQUESTS; j++){
        total_movement += abs(current_position - requests_sorted[j]);
        current_position = requests_sorted[j];
    }
    return total_movement;
}

/*
Circular SCAN. Moves towards one end then switches head position to the other and moves towards initial position.
*/
int CSCAN(int *requests, int *requests_sorted, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    int starting_index = findStartingIndex(requests_sorted, initial_position);
    // Move left
    for (int i = starting_index-1; i >= 0; i--){
        total_movement += abs(current_position - requests_sorted[i]);
        current_position = requests_sorted[i];
    }
    // To include the last bit between the wall and current position as well as the gap between the wall
    // and next position
    total_movement += abs(current_position - 0);
    total_movement += abs((DISK_SIZE-1) - requests_sorted[AMOUNT_OF_REQUESTS-1]);

    // Move to end of disk, then move towards the initial position
    for (int j = AMOUNT_OF_REQUESTS-1; j >= starting_index; j--){
        total_movement += abs(current_position - requests_sorted[j]);
        current_position = requests_sorted[j];
    }
    return total_movement;
}

/*
Same as scan, but does not go all the way to the side of the disk
*/
int Look(int *requests, int *requests_sorted, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    int starting_index = findStartingIndex(requests_sorted, initial_position);
    // Move left
    for (int i = starting_index-1; i >= 0; i--){
        total_movement += abs(current_position - requests_sorted[i]);
        current_position = requests_sorted[i];
    }
    
    // Move right
    for (int j = starting_index; j < AMOUNT_OF_REQUESTS; j++){
        total_movement += abs(current_position - requests_sorted[j]);
        current_position = requests_sorted[j];
    }
    
    return total_movement;
}

/*
Same as C-scan without going all the way to the disk sides.
*/
int CLook(int *requests, int *requests_sorted, int initial_position) {
    int total_movement = 0;
    int current_position = initial_position;
    int starting_index = findStartingIndex(requests_sorted, initial_position);
    // Move left
    for (int i = starting_index-1; i >= 0; i--){
        total_movement += abs(current_position - requests_sorted[i]);
        current_position = requests_sorted[i];
    }
    // Move to end of queue, then move towards the initial position
    for (int j = AMOUNT_OF_REQUESTS-1; j >= starting_index; j--){
        total_movement += abs(current_position - requests_sorted[j]);
        current_position = requests_sorted[j];
    }
    return total_movement;
}

/*
Checks if the string is a valid positive integer in the range of 0-4999. 
*/
int parseCmdArg(char *arg) {
    int errno_backup = errno;
    errno = 0; 
    char *endptr;
    long parsed_argument = strtol(arg, &endptr, 10);

    // Check for general parsing errors
    if (errno != 0) {
        printf("Error: Could not parse the argument!\n");
        exit(EXIT_FAILURE);
    }

    // Check if it is even a number
    if (*endptr != 0) {
        printf("Error: Malformed argument!\n");
        exit(EXIT_FAILURE);
    }

    // Check if argument is in range
    if (parsed_argument < 0 || parsed_argument > 4999) {
        printf("Error: Argument out of range [0, 4999]!\n");
        exit(EXIT_FAILURE); 
    }

    errno = errno_backup;
    return (int) parsed_argument;
}

/*
Needed for the qsort function. Reference: https://www.educative.io/answers/what-is-the-qsort-function-in-c
*/
int comparator(const void *p1, const void *p2) {
  return (*(int*)p1 - *(int*)p2);
}

/*
Finds the index where the value is larger than the specified value and the previous is lower.
*/
int findStartingIndex(int *array, int value) {
    int index = 0;
    while (value > array[index]) {
        index++;
    }
    return index;
}

/*
Prints the contents of an integer array
*/
void printArray (int *array, int size) {
    printf("\n----- Contents of the array ------\n");
    for (int i = 0; i < size; i++) {
        printf("Index: %d, Value: %d\n", i, array[i]);
    }
    printf("\n");
}

int shortestDistance(int low, int mid, int high) {
    return ((abs(low - mid) <= abs(mid - high))? low:high);
}