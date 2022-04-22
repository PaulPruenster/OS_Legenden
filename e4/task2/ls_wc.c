#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

/*
    Inspired by:
    https://serveanswer.com/questions/executing-the-ls-wc-linux-command-in-c-program-using-2-processes-communicating-trough-a-pipe
*/

void close_pipe(int* fd)
{
    close(fd[READ_END]);
    close(fd[WRITE_END]);
}

int main(void)
{
    int fd[2];

    if (pipe(fd) < 0) {
        printf("Pipe error");
        exit(1);
    }
    pid_t pid1 = fork();
    if (pid1 < 0) {
        printf("Fork error");
        exit(1);
    }
    if (pid1 == 0) // Child1
    {
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close_pipe(fd);

        // Uses the PATH variable, doesnt need the absolute path
        execlp("ls", "ls", NULL, NULL);
        exit(1);
    }

    // Parent
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        printf("Fork error");
        exit(1);
    }
    if (pid2 == 0) // Child2
    {
        dup2(fd[READ_END], STDIN_FILENO);
        close_pipe(fd);

        execlp("wc", "wc", "-l", NULL);
        exit(1);
    }

    // Parent
    close_pipe(fd);

    wait(NULL);
    return 0;
}