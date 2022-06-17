

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  pid_t cpid = fork();
  if (cpid == -1)
    return EXIT_FAILURE;
  if (cpid == 0) {
    // Child
    printf("Child PID = %d\n", getpid());
    exit(getpid());
  }
  // Parent
  printf("Parent PID = %d\n", getpid());

  while ((cpid = wait(NULL)) > 0) {
  }

  printf("Done\n");

  return EXIT_SUCCESS;
}