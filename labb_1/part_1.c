#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    // fd stands for file descriptors
    // fd[0] is to read from the pipe
    // fd[1] is to write from the pipe
    int fd[2];
    pid_t pid;

    // Create a pipe
    if (pipe(fd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Fork the process to create a child
    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  
        // Child process
        // Close the read end of the pipe, not needed in the child
        close(fd[0]);

        // Redirect stdout to the write end of the pipe
        // The `dup2` function duplicates an existing file descriptor to another file descriptor. 
        // If the target file descriptor (the second argument) is already open, `dup2` closes it 
        // before duplicating the file descriptor. After the call, both file descriptors refer to 
        // the same file, socket, or pipe, and they share file offset and file status flags.

        // STDOUT_FILENO: This is a macro that is defined in `<unistd.h>` header, and it refers 
        // to the file descriptor number of standard output, which is `1`. 
        // In Unix-like systems, the standard input, output, and error are represented by 
        // file descriptors `0`, `1`, and `2`, respectively.
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 redirect failed in child");
            exit(EXIT_FAILURE);
        }

        // Close the original write end of the pipe
        // Commented out since we don't think it is needed since dup2 closes it
        // close(fd[1]);

        // The exec() function replaces the program in the current process with a brand new program.
        // https://stackoverflow.com/questions/4204915/please-explain-the-exec-function-and-its-family
        // Replace the child process with the "ls /" command and gives the "ls" process the 
        // childs pid (process id)
        execlp("ls", "ls", "/", NULL);

        // execlp only returns on error
        perror("execlp failed in child");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;

        // Close the write end of the pipe, not needed in the parent
        close(fd[1]);

        // Wait for the child process to finish
        // Wait for a child to die. When one does, put its status 
        // in *status and return its process ID.
        pid = wait(&status);

        // Redirect stdin to the read end of the pipe
        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2 redirect failed in parent");
            exit(EXIT_FAILURE);
        }

        // Close the original read end of the pipe
        // Commented out since we don't think it is needed since dup2 closes it
        // close(fd[0]);

        // Replace the parent process with the "wc -l" command
        execlp("wc", "wc", "-l", NULL);

        // execlp only returns on error
        perror("execlp failed in parent");
        exit(EXIT_FAILURE);
    }

    // This should never be reached
    return EXIT_FAILURE;
}
