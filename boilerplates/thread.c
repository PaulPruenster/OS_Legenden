#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define N 100

typedef struct threadData
{
    int id;
    pthread_t thread;
} threadData;

void *thread_fun(void *arg)
{
    int i = *(int *)arg;
    printf("%d\n", i);
    return NULL;
}

int main(int argc, char **argv)
{
    // supress warnings for unused variables; 
    (void) argc; 
    (void) argv; 
    
    // create N threads
    threadData threads[N];
    for (int i = 0; i < N; i++)
    {
        threads[i].id = i;
        pthread_create(&threads[i].thread, NULL, thread_fun, (void *)&i);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i].thread, NULL); // last arg is return value
    }

    return EXIT_SUCCESS;
}