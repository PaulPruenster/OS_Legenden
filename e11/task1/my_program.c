#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: ./%s <number>\n", argv[0]);
    return EXIT_FAILURE;
  }
  printf("\n%d\n", atoi(argv[1]));
  return EXIT_SUCCESS;
}