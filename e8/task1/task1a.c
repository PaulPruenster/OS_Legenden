#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

#define NUM_THREADS 5000

atomic_int global = NUM_THREADS;

void *main_thread()
{
  atomic_fetch_sub(&global, 1);
  return NULL;
}

int main(void)
{
  printf("Before: %i\n", global);

  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; ++i)
    pthread_create(&threads[i], NULL, main_thread, NULL);
  for (int i = 0; i < NUM_THREADS; ++i)
    pthread_join(threads[i], NULL);

  printf("After: %i\n", global);
  return EXIT_SUCCESS;
}