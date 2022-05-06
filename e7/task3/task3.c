// https://en.cppreference.com/w/c/thread/call_once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define PLAYERS 5

struct SharedData
{
    int values[PLAYERS];
    pthread_barrier_t barrier;
    int i;
};
typedef struct SharedData SharedData;

struct ThreadData
{
    SharedData *data;
    int thread_id;
    pthread_t thread;
};
typedef struct ThreadData ThreadData;

void rollTheDice(ThreadData *data)
{
    // rand() % (max_number + 1 - minimum_number) + minimum_number
    data->data->values[data->data->i] = rand() % (6 + 1 - 1) + 1;
}

void computeLoser(ThreadData * data) {
    int min = 6; 
    for(int i = 0; i < PLAYERS; i++){
        if (data->data->values[i] != NULL && data->data->values[i] < min) {
            min = data->data->values[i];
        }
    }

    for(int i = 0; i < PLAYERS; i++){
        if(data->data->values[i] <= min) {
            data->data->values[i] = NULL;
        }
    }
}

void *thread(void *arg)
{
    ThreadData *thread_data = arg;
    SharedData *data = thread_data->data;

    rollTheDice(thread_data);

    if (pthread_barrier_wait(&data->barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        computeLoser(data);
    }

    int r = rand() % (6 + 1 - 1) + 1;
    printf("Player %d rolled a %d\n", r, r);
    return NULL;
}

int main()
{

    
    srand(time(NULL));
    SharedData data = {0};
    
    ThreadData thread_data[PLAYERS];

    if (pthread_barrier_init(&data.barrier, NULL, PLAYERS) != 0)
    {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < PLAYERS; i++)
    {
        thread_data[i].data = &data;
        thread_data[i].thread_id = i; 
        thread_data[i].data->i = i; 
        if (pthread_create(&thread_data[i].thread, NULL, thread, (void *)&i) != 0)
            return EXIT_FAILURE;
    }
    for (int i = 0; i < PLAYERS; i++)
    {
        pthread_join(&thread_data[i].thread, NULL);
    }
}
