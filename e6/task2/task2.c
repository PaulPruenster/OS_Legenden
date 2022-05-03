#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "myqueue.h"

int global = 0;

void *myThreadFun(void *vargp)
{
  int value = *((int *)vargp);
  global += value;

  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s <list of numbers>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int i;

  for (i = 1; i < argc; i++)
  {
    int val = atoi(argv[i]);
    pthread_t tid;
    pthread_create(&tid, NULL, myThreadFun, (void *)&val);

    pthread_join(tid, NULL);
    printf("sum%d = %d\n", i, global);
  }

  pthread_exit(NULL);
  return 0;
}
