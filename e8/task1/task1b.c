#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "thpool.h"

#define THREADS 5
#define NUMBERS 50000

// https://nachtimwald.com/2019/04/12/thread-pool-in-c/
int global = NUMBERS;

void worker()
{
  global--;
}

int main()
{

  threadpool pool = thpool_init(THREADS);

  for (int i = 0; i < NUMBERS; i++)
    thpool_add_work(pool, worker, NULL);

  thpool_wait(pool);

  printf("Final: %d\n", global);

  thpool_destroy(pool);
  return 0;
}
