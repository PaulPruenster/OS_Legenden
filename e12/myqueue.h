#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/queue.h>

typedef struct myqueue_entry
{
  int value;
  pthread_cond_t *client_notification;
  pthread_mutex_t *client_mutex;
  STAILQ_ENTRY(myqueue_entry)
  entries;
}myqueue_entry;

STAILQ_HEAD(myqueue_head, myqueue_entry);

typedef struct myqueue_head myqueue;

static void myqueue_init(myqueue *q)
{
  STAILQ_INIT(q);
}

static bool myqueue_is_empty(myqueue *q)
{
  return STAILQ_EMPTY(q);
}

static void myqueue_push(myqueue *q, struct myqueue_entry *values)
{
  STAILQ_INSERT_TAIL(q, values, entries);
}

static void *myqueue_pop(myqueue *q)
{
  assert(!myqueue_is_empty(q));
  struct myqueue_entry *entry = STAILQ_FIRST(q);
  // const int value = entry->value;
  STAILQ_REMOVE_HEAD(q, entries);
  // free(entry);
  // return value;
  return entry;
}

#endif
