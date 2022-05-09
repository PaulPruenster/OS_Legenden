#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "myqueue.h"
#include <stdint.h>
#include <stdatomic.h>
#include <math.h>

#define CHILDREN 2
#define MAXITER pow(10, 6)

#if USE_MY_MUTEX
#define my_mutex_lock(m) while (atomic_flag_test_and_set(m))
#define my_mutex_unlock(m) atomic_flag_clear(m)
#define MUTEX_LOCK my_mutex_lock
#define MUTEX_UNLOCK my_mutex_unlock

#else
#define MUTEX_LOCK pthread_mutex_lock
#define MUTEX_UNLOCK pthread_mutex_unlock

#endif

myqueue *q;
pthread_cond_t cond;
pthread_mutex_t mutex;

void *myThreadFun(void *vargp)
{
    int val = -1, sum = 0;
    bool b = true;
    while (b)
    {
        MUTEX_LOCK(&mutex);
        if (!myqueue_is_empty(q))
        {
            // pthread_cond_wait(&cond, &mutex);
            val = myqueue_pop(q);
            sum += val;
            if (val == 0)
            {
                b = false;
            }
        }

        MUTEX_UNLOCK(&mutex);
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
    pthread_cond_init(&cond, NULL);
    // pthread_mutex_init(&mutex, NULL);

    pthread_t threads[CHILDREN];
    for (int i = 0; i < CHILDREN; i++)
    {
        int *p = malloc(sizeof(void *));
        *p = i;
        pthread_create(&threads[i], NULL, myThreadFun, (void *)p);
    }

    for (size_t i = 0; i < MAXITER; i++)
    {
        MUTEX_LOCK(&mutex);
        myqueue_push(q, 1);
        MUTEX_UNLOCK(&mutex);
        // pthread_cond_signal(&cond);
    }
    for (size_t i = 0; i < CHILDREN; i++)
    {
        MUTEX_LOCK(&mutex);
        myqueue_push(q, 0);
        MUTEX_UNLOCK(&mutex);
        // pthread_cond_signal(&cond);
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
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    return 0;
}
