#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
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
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 redirect failed in child");
            exit(EXIT_FAILURE);
        }

        // Close the original write end of the pipe
        close(fd[1]);

        // Replace the child process with the "ls /" command
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
        waitpid(pid, &status, 0);

        // Redirect stdin to the read end of the pipe
        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2 redirect failed in parent");
            exit(EXIT_FAILURE);
        }

        // Close the original read end of the pipe
        close(fd[0]);

        // Replace the parent process with the "wc -l" command
        execlp("wc", "wc", "-l", NULL);

        // execlp only returns on error
        perror("execlp failed in parent");
        exit(EXIT_FAILURE);
    }

    // This should never be reached
    return EXIT_FAILURE;
}
