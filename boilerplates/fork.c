#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

double mc_pi(int64_t S) {
  int64_t in_count = 0;
  for (int64_t i = 0; i < S; ++i) {
    const double x = rand() / (double)RAND_MAX;
    const double y = rand() / (double)RAND_MAX;
    if (x * x + y * y <= 1.f) {
      in_count++;
    }
  }
  return 4 * in_count / (double)S;
}

int main(int argc, char *argv[]) {
  if (argc != 3)
    return EXIT_FAILURE;

  size_t n = atoi(argv[1]);
  if ((n == 0 && argv[1][0] != '0') || argv[1][0] == '-')
    return EXIT_FAILURE;

  int s = atoi(argv[2]);
  if (s == 0 && argv[2][0] != '0')
    return EXIT_FAILURE;

  pid_t cpid;
  for (size_t i = 0; i < n; i++) {
    cpid = fork();
    srand(getpid());
    if (cpid == -1)
      return EXIT_FAILURE;
    if (cpid == 0) {
      printf("Child %ld PID = %d. mc_pi(%d) = %lf\n", i, getpid(), s, mc_pi(s));
      exit(getpid());
    }
  }
  int status = 0;
  while ((cpid = wait(&status)) > 0) {
  }

  write(1, "Done\n", 6);

  return EXIT_SUCCESS;
}