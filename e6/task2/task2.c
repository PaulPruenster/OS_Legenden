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
int global = 0;

typedef struct thread_data
{
  int index;
  myqueue *q;
} thread_data;

void *myThreadFun(void *vargp)
{
  thread_data *td = (thread_data *)vargp;
  myqueue *q = (myqueue *)td->q;

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

  global += sum;

  printf("Consumer %i sum :%d\n", td->index, sum);
  return NULL;
}

int main()
{
  myqueue *q = malloc(sizeof(myqueue));
  myqueue_init(q);

  pthread_t threads[CHILDREN];
  for (int i = 0; i < CHILDREN; i++)
  {
    thread_data *td = malloc(sizeof(thread_data));
    td->index = i;
    td->q = q;

    pthread_create(&threads[i], NULL, myThreadFun, (void *)td);
  }

  for (size_t i = 0; i < 100000; i++)
  {
    myqueue_push(q, 1);
  }
  for (size_t i = 0; i < CHILDREN; i++)
  {
    myqueue_push(q, 0);
  }

  for (int i = 0; i < CHILDREN; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("Final sum: %d\n", global);
  fflush(stdout);
  pthread_exit(NULL);
  return 0;
}
