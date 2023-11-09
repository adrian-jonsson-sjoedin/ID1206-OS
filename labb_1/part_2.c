// Compile the program using gcc and pass -lrt to link the real-time library,
// which is required for message queues:
// gcc program.c -o program -lrt

// To run the program, provide the path to a text file as an argument:
// ./program textfile.txt

// Please note that in this code, the message queue name, buffer size, and permissions
// are defined as constants. Depending on the size of the text file and the system limitations,
// you may need to adjust the MAX_SIZE value accordingly.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <mqueue.h>


#define QUEUE_NAME "/my_queue"
#define MAX_SIZE 4096
#define PERMISSIONS 0660

// counts the number of words in the string sent to it.
// '*', a pointer, means here is what I want you to look at
int word_count(const char *string) {
    int count = 0;
    int in_word = 0;
    // Goes through character by character and checks if it is a new word or not and if so, 
    // counts the word.
    // If the character is part of the same word, it continues on to the next character.
    // If the character is either a blank space, a new line, or a tab, it expects a new 
    // word to be next.
    while (*string) {
        if (*string == ' ' || *string == '\n' || *string == '\t') {
            in_word = 0;
        } else if (in_word == 0) {
            in_word = 1;
            count++;
        }
        string++;
    }
    return count;
}

int main(int argc, char *argv[]) {
    // argc stands for argument count and argv stands for argument values.
    // These are variables passed to the main function when it starts executing.
    // The executable (./a.out file) counts as one argument.
    if (argc != 2) { // If we don't pass two arguments then we give this message to the user.
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    pid_t pid; //proccess id
    mqd_t mq; // message queue descriptors. This will not be an array type. A message queue descriptor may be implemented using a file descriptor
    struct mq_attr attr; // attributes of a message que. See slide 49 lecture 2.
    char buffer[MAX_SIZE + 1];
    
    // Set the attributes for the message queue
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    // Create the message queue
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, PERMISSIONS, &attr);
    if (mq == (mqd_t)-1) { //mqd_t is a cast
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    pid = fork(); // create child and parent process
    if (pid == -1) { // check for any errors in creation
        perror("fork");
        mq_unlink(QUEUE_NAME); //remove the message que
        exit(EXIT_FAILURE);


    } else if (pid > 0) { // check if parent process
        // Parent process is responsible for reading the file to a buffer and save it to 
        // the message queue
        int status;
        FILE *file = fopen(argv[1], "r"); //argv[1] is the second argument given, i.e. the file
        if (file == NULL) { // if error with opening the file, terminate
            perror("fopen");
            mq_unlink(QUEUE_NAME);
            exit(EXIT_FAILURE);
        }

        // Read the contents of the file into the buffer
        size_t bytes_read = fread(buffer, 1, MAX_SIZE, file);
        buffer[bytes_read] = '\0'; // Ensure null-termination

        // Close the stream
        fclose(file);

        // Send the contents of the file to the message queue
        if (mq_send(mq, buffer, strlen(buffer), 0) == -1) {
            // If something went wrong
            perror("mq_send");
            mq_unlink(QUEUE_NAME);
            exit(EXIT_FAILURE);
        }

        // Wait for the child to finish
        pid = wait(&status);

        // Close and unlink the message queue
        mq_close(mq);
        mq_unlink(QUEUE_NAME);


    } else {// Child process
        ssize_t bytes_received;

        // Receive the message from the message queue
        bytes_received = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_received == -1) {
            perror("mq_receive");
            mq_close(mq);
            exit(EXIT_FAILURE);
        }
        buffer[bytes_received] = '\0'; // Ensure null-termination

        // Count the words in the received message
        int count = word_count(buffer);

        // Output the word count to the terminal
        printf("The number of words in the file %s is: %d\n", argv[1], count);

        // Close the message queue
        mq_close(mq);
    }

    return 0;
}