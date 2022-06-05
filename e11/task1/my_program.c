#include <stdio.h>
#include <stdlib.h>

int square_my_integer(int x) { return x * x; }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s <number>\n", argv[0]);
    return EXIT_FAILURE;
  }
  printf("%d\n", square_my_integer(atoi(argv[1])));
  return EXIT_SUCCESS;
}