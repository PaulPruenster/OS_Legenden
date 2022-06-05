#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
// source: https://jameshfisher.com/2017/08/24/dlopen/
typedef int plugin_func(int);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Not enough arguments!\n");
    return EXIT_FAILURE;
  }
  int number = atoi(argv[1]);

  for (int i = 2; i < argc; i++) {
    void *handle = dlopen(argv[i], RTLD_LAZY);
    if (handle == NULL) {
      fprintf(stderr, "Could not open plugin: %s\n", dlerror());
      return EXIT_FAILURE;
    }
    plugin_func *f = dlsym(handle, "plugin_func");
    if (f == NULL) {
      fprintf(stderr, "Could not find plugin_func: %s\n", dlerror());
      return EXIT_FAILURE;
    }

    number = f(number);
    printf("%s: %d\n", argv[i], number);

    if (dlclose(handle) != 0) {
      fprintf(stderr, "Could not close plugin: %s\n", dlerror());
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}