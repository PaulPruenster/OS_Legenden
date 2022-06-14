#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "myqueue.h"
pthread_mutex_t counter_mutex;
atomic_int counter = -1;

pthread_mutex_t mutex;
pthread_cond_t cook_cond;

atomic_int client_counter;
atomic_int order_counter;
myqueue *q;


int notification = 0;

void *cook_thread(void *vargp)
{
    int counter_is_not_empty;
    int id = 0;
    id = *(int *)vargp;
    myqueue_entry *val = {0};
    while (1)
    {
        pthread_mutex_lock(&mutex);
        // Loop is needed, because pthread_cond_wait does randomly continue sometimes
        while (myqueue_is_empty(q))
        {
            pthread_cond_wait(&cook_cond, &mutex);
        }
        val = myqueue_pop(q);
        pthread_mutex_unlock(&mutex);

        if (val->value == -1)
        {
            free(vargp);
            return NULL;
        }

        printf("Cook %d is preparing order %d\n", id, val->value);
        int rand_time = (rand() % (500 - 100 + 1)) + 100;
        usleep(rand_time * 1000);
        //  Loop is needed, because pthread_cond_wait does randomly continue sometimes
        counter_is_not_empty = 1;

        while (counter_is_not_empty)
        {
            pthread_mutex_lock(&counter_mutex);
            if (counter == -1)
            {
                printf("Cook %d has placed order %d on counter\n", id, val->value);

                counter = val->value;
                counter_is_not_empty = 0;
                // pthread_mutex_lock(val->client_mutex);
                if (notification)
                {
                    pthread_cond_signal(val->client_notification);
                }
                // pthread_mutex_unlock(val->client_mutex);
            }

            pthread_mutex_unlock(&counter_mutex);
        }
    }

    free(vargp);
    return NULL;
}

void *guest_function(void *arg)
{

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ms1 = ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
    int id = *(int *)arg;
    free(arg);
    int order_id = order_counter++;

    myqueue_entry *entry = malloc(sizeof(myqueue_entry));

    entry->value = order_id;
    entry->client_notification = NULL;
    entry->client_mutex = NULL;
    // pthread_cond_t *cond_var;
    if (notification)
    {
        entry->client_notification = malloc(sizeof(pthread_cond_t));
        pthread_cond_init(entry->client_notification, NULL);
        entry->client_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(entry->client_mutex, NULL);
        // entry->client_notification = cond_var;
    }

    printf("Guest %d has made meal order %d\n", id, order_id);

    pthread_mutex_lock(&mutex);
    myqueue_push(q, entry);
    pthread_cond_signal(&cook_cond);
    pthread_mutex_unlock(&mutex);
    if (notification)
    {
        pthread_mutex_lock(entry->client_mutex);

        while (counter != order_id)
        {
            pthread_cond_wait(entry->client_notification, entry->client_mutex);
        }
        pthread_mutex_unlock(entry->client_mutex);
        counter = -1;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint64_t ms2 = ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
        printf("Guest %d has picked up order %d after %ld ms\n", id, order_id, ms2 - ms1);
        // pthread_mutex_unlock(&counter_mutex);
        uint64_t *ret = malloc(sizeof(uint64_t));
        *ret = ms2 - ms1;
        pthread_cond_destroy(entry->client_notification);
        pthread_mutex_destroy(entry->client_mutex);
        free(entry->client_mutex);
        free(entry->client_notification);
        free(entry);
        return (void *)ret;
    }
    else
    {
        int found = 1;
        while (found)
        {
            usleep(100 * 1000);
            pthread_mutex_lock(&counter_mutex);
            if (counter != -1)
            {
                usleep(100 * 1000);
                if (counter == order_id)
                {
                    found = 0;
                    counter = -1;
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    uint64_t ms2 = ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
                    printf("Guest %d has picked up order %d after %ld ms\n", id, order_id, ms2 - ms1);
                    pthread_mutex_unlock(&counter_mutex);
                    uint64_t *ret = malloc(sizeof(uint64_t));
                    *ret = ms2 - ms1;
                    free(entry);
                    return (void *)ret;
                }
            }
            pthread_mutex_unlock(&counter_mutex);
        }
    }
    free(entry);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("./restaurant <enable notifications> <number of guests> <number of cooks>\n");
        return EXIT_FAILURE;
    }

    notification = atoi(argv[1]);
    int number_guest = atoi(argv[2]);
    int number_cooks = atoi(argv[3]);

    q = malloc(sizeof(myqueue));
    myqueue_init(q);
    pthread_cond_init(&cook_cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&counter_mutex, NULL);

    srand(time(NULL));

    pthread_t cooks[number_cooks];
    for (int i = 0; i < number_cooks; i++)
    {
        int *p = malloc(sizeof(int *));
        *p = i;
        pthread_create(&cooks[i], NULL, cook_thread, p);
    }

    pthread_t guests[number_guest];

    for (int i = 0; i < number_guest; i++)
    {
        int *p = malloc(sizeof(int *));
        *p = i;
        pthread_create(&guests[i], NULL, guest_function, p);
    }

    uint64_t total_sum = 0;
    for (int i = 0; i < number_guest; i++)
    {
        int *child_sum;
        pthread_join(guests[i], (void **)&child_sum);
        int a = *child_sum;
        free(child_sum);
        total_sum += a;
    }

    //STOP = 0;
    for(int i = 0; i < number_cooks;i++){
        myqueue_entry *entry = malloc(sizeof(myqueue_entry));
        entry->value = -1;
        entry->client_notification = NULL;
        entry->client_mutex = NULL;
        pthread_mutex_lock(&mutex);
        myqueue_push(q, entry);
        pthread_cond_signal(&cook_cond);
        pthread_mutex_unlock(&mutex);
    }

    for (int i = 0; i < number_cooks; i++)
    {
        //pthread_cancel(cooks[i]);
        pthread_join(cooks[i],NULL);
    }
    pthread_cond_destroy(&cook_cond);
    pthread_mutex_destroy(&mutex);
    printf("All guests habe veen served with an average wait time of %ld ms\n", (total_sum / number_guest));

    free(q);
    return EXIT_SUCCESS;
}