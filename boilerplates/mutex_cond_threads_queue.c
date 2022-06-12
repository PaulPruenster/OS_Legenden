#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "myqueue.h"
#include <stdint.h>

// The following code is from my solution to exercise 6.3
#define CHILDREN 500
#define MAXITER 100000

typedef struct thread_data
{
    myqueue *q;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} data;

typedef struct threads_info
{
    data *my_data;
    int id;
} info;



void *myThreadFun(void *vargp)
{
    info *thread_info = (info *)vargp;
    data *my_data = thread_info->my_data;
    int val = -1, sum = 0;
    bool b = true;
    while (b)
    {
        pthread_mutex_lock(&my_data->mutex);
        // Loop is needed, because pthread_cond_wait does randomly continue somethimes
        while (myqueue_is_empty(my_data->q))
        {
            pthread_cond_wait(&my_data->cond, &my_data->mutex);
        }
        val = myqueue_pop(my_data->q);
        sum += val;
        if (val == 0)
        {
            b = false;
        }

        pthread_mutex_unlock(&my_data->mutex);
    }
    int *ret = malloc(sizeof(int));
    *ret = sum;

    printf("Consumer %d sum: %d\n", thread_info->id, sum);
    free(vargp);
    return (void *)ret;
}

int main()
{
    data *my_data = malloc(sizeof(data));
    my_data->q = malloc(sizeof(myqueue));
    // q = malloc(sizeof(myqueue));
    myqueue_init(my_data->q);
    pthread_cond_init(&(my_data->cond), NULL);
    pthread_mutex_init(&(my_data->mutex), NULL);

    pthread_t threads[CHILDREN];
    for (int i = 0; i < CHILDREN; i++)
    {
        info *infos = malloc(sizeof(info));
        infos->my_data = my_data;
        infos->id = i;
        // int *p = malloc(sizeof(void *));
        //*p = i;
        pthread_create(&threads[i], NULL, myThreadFun, (void *)infos);
    }

    for (size_t i = 0; i < MAXITER; i++)
    {
        pthread_mutex_lock(&(my_data->mutex));
        myqueue_push(my_data->q, 1);
        pthread_mutex_unlock(&(my_data->mutex));
        pthread_cond_signal(&(my_data->cond));
    }
    for (size_t i = 0; i < CHILDREN; i++)
    {
        pthread_mutex_lock(&(my_data->mutex));
        myqueue_push(my_data->q, 0);
        pthread_mutex_unlock(&(my_data->mutex));
        pthread_cond_signal(&(my_data->cond));
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
    pthread_cond_destroy(&(my_data->cond));
    pthread_mutex_destroy(&(my_data->mutex));
    free(my_data);
    return 0;
}
