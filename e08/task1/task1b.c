#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "myqueue.h"

#define THREADS 5
#define NUMBERS 50000
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; // better without global(s)
typedef struct thread_pool
{
  bool running;
  pthread_t *threads;
  myqueue *q;
  int thread_count;
} thread_pool;

typedef myqueue_entry *job_id;

void *myThreadFun(void *vargp)
{
  thread_pool *pool = (thread_pool *)vargp;

  while (pool->running)
  {
    pthread_mutex_lock(&mut);
    if (!myqueue_is_empty(pool->q))
    {
      myqueue_entry *entry = myqueue_pop(pool->q);

      entry->job_fun(entry->arg); // call the function like job_fun(arg)
      entry->arg = NULL;
    }
    pthread_mutex_unlock(&mut);
  }
  return NULL;
}

void pool_create(thread_pool *pool, size_t size)
{
  if (pool == NULL || size < 1)
  {
    perror("Could not create pool!\n");
    return;
  }

  // create the queue
  myqueue *q = malloc(sizeof(myqueue));
  if (q == NULL)
  {
    perror("Could not create queue!\n");
    return;
  }
  myqueue_init(q);
  pool->q = q;
  pool->running = true;

  // create workers that consume the jobs
  pthread_t *workers = malloc(sizeof(pthread_t) * size);
  if (workers == NULL)
  {
    perror("Could not create worker array!\n");
    return;
  }
  for (size_t i = 0; i < size; i++)
    pthread_create(&workers[i], NULL, myThreadFun, pool);

  pool->threads = workers;
  pool->thread_count = size;
}

// make a myqueentry with the job and the atmoic pointer and return the myqueentry pointer as id
job_id pool_submit(thread_pool *pool, job_function start_routine, job_arg arg)
{
  myqueue_entry *entry = malloc(sizeof(myqueue_entry));
  entry->arg = arg;
  entry->job_fun = start_routine;

  pthread_mutex_lock(&mut);
  myqueue_push(pool->q, entry);
  pthread_mutex_unlock(&mut);
  return entry;
}

void pool_await(job_id id)
{
  while (id->arg != NULL)
    ;
  free(id);
  // better with semaphore
  // thread to thread semaphore, for mutex undefined behaviour 
}

void pool_destroy(thread_pool *pool)
{
  // cancel all running threads
  pool->running = false;
  for (int i = 0; i < pool->thread_count; i++)
      pthread_join(pool->threads[i], NULL);

  // free the struct
  free(pool->threads);
  free(pool->q);
  free(pool);
}
// job to be pushed to q
void *job(void *arg)
{
  atomic_fetch_sub((atomic_int *)arg, 1);
  return NULL;
}

int main()
{
  atomic_int number = NUMBERS;

  thread_pool *pool = malloc(sizeof(thread_pool));
  pool_create(pool, THREADS);

  job_id ids[NUMBERS];
  for (int i = 0; i < NUMBERS; i++)
    ids[i] = pool_submit(pool, job, &number);

  for (int i = 0; i < NUMBERS; i++)
    pool_await(ids[i]);

  printf("Final: %d\n", number);

  pool_destroy(pool);
  return 0;
}
