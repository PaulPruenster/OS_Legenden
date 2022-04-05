#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

void close_pipes(int* p1, int* p2)
{
    close(p1[READ_END]);
    close(p1[WRITE_END]);
    close(p2[READ_END]);
    close(p2[WRITE_END]);
}

int main(void)
{
    int p1[2];
    int p2[2];

    if (pipe(p1) < 0) {
        printf("Pipe 1 error");
        exit(1);
    }
    if (pipe(p2) < 0) {
        printf("Pipe 2 error");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        printf("Fork error");
        exit(1);
    }
    if (pid1 == 0) { // Child1
        dup2(p1[WRITE_END], STDOUT_FILENO);
        close_pipes(p1, p2);
 
        // Uses the PATH variable, doesnt need the absolute path
        execlp("ls", "ls", NULL, NULL);
        exit(1);
    }

    // Parent
    pid_t pid2 = fork();
    if (pid2 < 0) {
        printf("Fork error");
        exit(1);
    }
    if (pid2 == 0) { // Child1
        dup2(p1[READ_END], STDIN_FILENO);
        dup2(p2[WRITE_END], STDOUT_FILENO);
        close_pipes(p1, p2);

        // Uses the PATH variable, doesnt need the absolute path
        execlp("grep", "grep", "-v", "lab");
        exit(1);
    }

    // Parent
    pid_t pid3 = fork();
    if (pid3 < 0) {
        printf("Fork error");
        exit(1);
    }
    if (pid3 == 0) { // Child2
        dup2(p2[READ_END], STDIN_FILENO);
        close_pipes(p1, p2);

        execlp("wc", "wc", "-l", NULL);
        exit(1);
    }

    // Parent
    close_pipes(p1, p2);

    wait(NULL); // TODO waitpid? olle 3
    return 0;
}