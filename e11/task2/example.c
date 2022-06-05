#include <dlfcn.h>
#include <gnu/lib-names.h> /* Defines LIBM_SO (which will be a string such as "libm.so.6") */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  void *handle;
  double (*cosine)(double);
  char *error;

  handle = dlopen(LIBM_SO, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }

  dlerror(); /* Clear any existing error */

  cosine = (double (*)(double))dlsym(handle, "cos");

  error = dlerror();
  if (error != NULL) {
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
  }

  printf("%f\n", (*cosine)(2.0));
  dlclose(handle);
  exit(EXIT_SUCCESS);
}