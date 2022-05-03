#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "myqueue.h"

#define CHILDREN 5

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void *myThreadFun(void *vargp)
{
  myqueue *q = (myqueue *)vargp;

  int val = -1, sum = 0;
  bool b = true;
  while (b)
  {
    pthread_mutex_lock(&mut);
    if (!myqueue_is_empty(q))
    {
      val = myqueue_pop(q);
      fflush(stdout);
      sum += val;
      if (val == 0)
      {
        b = false;
      }
    }
    pthread_mutex_unlock(&mut);
  }
  // printf("Sum %d\n", sum);
  void *ret = malloc(sizeof(void *));
  ret = (void *)(&sum);

  printf("Pointer after cast :%d\n", *((int *)ret));
  return ret;
}

int main()
{
  myqueue *q = malloc(sizeof(myqueue));
  myqueue_init(q);

  pthread_t threads[CHILDREN];
  for (int i = 0; i < CHILDREN; i++)
  {
    pthread_create(&threads[i], NULL, myThreadFun, (void *)q);
  }

  for (size_t i = 0; i < 100000; i++)
  {
    myqueue_push(q, 1);
  }
  for (size_t i = 0; i < CHILDREN; i++)
  {
    myqueue_push(q, 0);
  }

  int allsum = 0;
  for (int i = 0; i < CHILDREN; i++)
  {
    void *child_sum;
    pthread_join(threads[i], &child_sum);
    allsum += *((int *)child_sum);
    printf("sum_child %d, all %d\n", *((int *)child_sum), allsum);
  }

  printf("Total sum: %d\n", allsum);
  fflush(stdout);
  pthread_exit(NULL);
  return 0;
}
