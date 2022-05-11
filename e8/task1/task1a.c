#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>

#define NUM_THREADS 30000

void *thread(void *arg)
{
  atomic_fetch_sub((atomic_int *)arg, 1);
  return NULL;
}

void printError()
{
  perror("Not enougth pthreads available!\n");
  exit(EXIT_FAILURE);
}

int main()
{
  atomic_int number = NUM_THREADS;

  printf("Before: %i\n", number);

  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; ++i)
    if (pthread_create(&threads[i], NULL, thread, &number) != 0)
      printError();

  for (int i = 0; i < NUM_THREADS; ++i)
    if (pthread_join(threads[i], NULL) != 0)
      printError();

  printf("After: %i\n", number);
  return EXIT_SUCCESS;
}