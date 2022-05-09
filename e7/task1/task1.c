#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

int global_var = 100000;
atomic_int atomic_counter = 100000;
pthread_mutex_t mutex;

// NOTE: to run in on ZID, you need to run 'module load gcc/9.2.0' first in the terminal and then compile it

void *thread(void *param)
{
    for (int i = 0; i < 10000; i++)
    {
        (void)param;
        // uncomment the following lines to use the implementation with mutex

        /* pthread_mutex_lock(&mutex);
        global_var--;
        pthread_mutex_unlock(&mutex); */

        // atomics:
        atomic_fetch_sub(&atomic_counter, 1);
    }
    return NULL;
}

int main()
{

    pthread_t threads[1000];
    pthread_mutex_init(&mutex, NULL);
    // create threads
    for (int i = 0; i < 1000; i++)
    {
        pthread_create(&threads[i], NULL, thread, NULL);
    }
    for (int i = 0; i < 1000; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // printf("%d", global_var);
    printf("\n%d", atomic_counter);
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
