
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
soruce:
https://stackoverflow.com/questions/262439/create-a-wrapper-function-for-malloc-and-free-in-c

command:
LD_PRELOAD=./malloc_spy.so ls
*/

void print_number(size_t number) {
  if (number > 9) {
    print_number(number / 10);
  }
  const char digit = '0' + number % 10;
  write(STDOUT_FILENO, &digit, 1);
}

void *malloc(size_t sz) {
  void *(*libc_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");

  write(STDOUT_FILENO, "allocating ", 11);
  print_number(sz);
  write(STDOUT_FILENO, " bytes\n", 7);

  return libc_malloc(sz);
}

int main() {
  free(malloc(10));
  return 0;
}