#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/queue.h>

typedef void *(*job_function)(void *);
typedef void *job_arg;

typedef struct myqueue_entry {
  job_function job_fun;
  job_arg arg;
  STAILQ_ENTRY(myqueue_entry)
  entries;
} myqueue_entry;

STAILQ_HEAD(myqueue_head, myqueue_entry);

typedef struct myqueue_head myqueue;

static void myqueue_init(myqueue *q) { STAILQ_INIT(q); }

static bool myqueue_is_empty(myqueue *q) { return STAILQ_EMPTY(q); }

static void myqueue_push(myqueue *q, myqueue_entry *value) {
  STAILQ_INSERT_TAIL(q, value, entries);
}

static myqueue_entry *myqueue_pop(myqueue *q) {
  assert(!myqueue_is_empty(q));
  struct myqueue_entry *entry = STAILQ_FIRST(q);
  STAILQ_REMOVE_HEAD(q, entries);
  // free(entry);
  return entry;
}

#endif
