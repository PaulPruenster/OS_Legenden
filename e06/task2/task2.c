#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "myqueue.h"
#include <stdint.h>

#define CHILDREN 5
#define MAXITER 100000

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
myqueue *q;

void *myThreadFun(void *vargp)
{
  int val, sum = 0;
  bool b = true;
  while (b)
  {
    pthread_mutex_lock(&mut);
    if (!myqueue_is_empty(q))
    {
      val = myqueue_pop(q);
      sum += val;
      if (val == 0)
      {
        b = false;
      }
    }
    pthread_mutex_unlock(&mut);
  }
  int *ret = malloc(sizeof(int));
  *ret = sum;

  printf("Consumer %d sum: %d\n", *(int *)vargp, sum);
  free(vargp);
  return (void *)ret;
}

int main()
{
  q = malloc(sizeof(myqueue));
  myqueue_init(q);

  pthread_t threads[CHILDREN];
  for (int i = 0; i < CHILDREN; i++)
  {
    int *p = malloc(sizeof(void *));
    *p = i;
    pthread_create(&threads[i], NULL, myThreadFun, (void *)p);
  }

  for (size_t i = 0; i < MAXITER; i++)
  {
    pthread_mutex_lock(&mut);
    myqueue_push(q, 1);
    pthread_mutex_unlock(&mut);
  }
  for (size_t i = 0; i < CHILDREN; i++)
  {
    pthread_mutex_lock(&mut);
    myqueue_push(q, 0);
    pthread_mutex_unlock(&mut);
  }

  int allsum = 0;
  for (int i = 0; i < CHILDREN; i++)
  {
    int *child_sum;
    pthread_join(threads[i], (void **)&child_sum);

    int a = *child_sum;
    free(child_sum);
    allsum += a;
  }

  printf("Final sum: %d\n", allsum);
  fflush(stdout);
  pthread_exit(NULL);
  return 0;
}
