#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

// Let us create a global variable to change it in threads
int g = 0;

// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
  int value = *((int *)vargp);
  g += value;

  // printf("Value %d, Sum %d\n", value, g);
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

  // Let us create three threads
  for (i = 1; i < argc; i++)
  {
    int val = atoi(argv[i]);
    pthread_t tid;
    pthread_create(&tid, NULL, myThreadFun, (void *)&val);

    pthread_join(tid, NULL);
    printf("sum%d = %d\n", i, g);
  }

  pthread_exit(NULL);
  return 0;
}
