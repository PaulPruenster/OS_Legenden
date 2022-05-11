#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// https://nachtimwald.com/2019/04/12/thread-pool-in-c/

static const size_t num_threads = 5;
static const size_t num_items = 1000;

int global = 0;

typedef void *(*thread_func_t)(void *arg);

struct tpool_work
{
  thread_func_t func;
  void *arg;
  struct tpool_work *next;
};
typedef struct tpool_work tpool_work_t;

struct tpool
{
  tpool_work_t *work_first;
  tpool_work_t *work_last;
  pthread_mutex_t work_mutex;
  pthread_cond_t work_cond;
  pthread_cond_t working_cond;
  size_t working_cnt;
  size_t thread_cnt;
  bool stop;
};
typedef struct tpool tpool_t;

void *worker()
{
  global++;
  printf("global=%d\n", global);

  return NULL;
}

tpool_work_t *tpool_work_create(thread_func_t func, void *arg)
{
  tpool_work_t *work;

  if (func == NULL)
    return NULL;

  work = malloc(sizeof(*work));
  work->func = func;
  work->arg = arg;
  work->next = NULL;
  return work;
}

void tpool_work_destroy(tpool_work_t *work)
{
  if (work == NULL)
    return;
  free(work);
}

tpool_t *pool_create(size_t num)
{
  tpool_t *tm;
  pthread_t thread;
  size_t i;

  if (num == 0)
    num = 2;

  tm = calloc(1, sizeof(*tm));
  tm->thread_cnt = num;

  pthread_mutex_init(&(tm->work_mutex), NULL);
  pthread_cond_init(&(tm->work_cond), NULL);
  pthread_cond_init(&(tm->working_cond), NULL);

  tm->work_first = NULL;
  tm->work_last = NULL;

  for (i = 0; i < num; i++)
  {
    pthread_create(&thread, NULL, worker, tm);
    pthread_detach(thread);
  }

  return tm;
}

bool pool_submit(tpool_t *tm, thread_func_t func, void *arg)
{
  tpool_work_t *work;

  if (tm == NULL)
    return false;

  work = tpool_work_create(func, arg);
  if (work == NULL)
    return false;

  pthread_mutex_lock(&(tm->work_mutex));
  if (tm->work_first == NULL)
  {
    tm->work_first = work;
    tm->work_last = tm->work_first;
  }
  else
  {
    tm->work_last->next = work;
    tm->work_last = work;
  }

  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  return true;
}

void pool_wait(tpool_t *tm)
{
  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  while (1)
  {
    if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0))
    {
      pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
    }
    else
    {
      break;
    }
  }
  pthread_mutex_unlock(&(tm->work_mutex));
}

void pool_destroy(tpool_t *tm)
{
  tpool_work_t *work;
  tpool_work_t *work2;

  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  work = tm->work_first;
  while (work != NULL)
  {
    work2 = work->next;
    tpool_work_destroy(work);
    work = work2;
  }
  tm->stop = true;
  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  pool_wait(tm);

  pthread_mutex_destroy(&(tm->work_mutex));
  pthread_cond_destroy(&(tm->work_cond));
  pthread_cond_destroy(&(tm->working_cond));

  free(tm);
}

int main()
{
  size_t i;

  tpool_t *tm = pool_create(num_threads);

  for (i = 0; i < num_items; i++)
    pool_submit(tm, worker, NULL);

  pool_wait(tm);

  printf("Final: %d\n", global);

  pool_destroy(tm);
  return 0;
}
