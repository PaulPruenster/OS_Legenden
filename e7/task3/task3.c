#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define PLAYER_LOST -1
#define PLAYERS 5

// https://godbolt.org/z/3YdsEvPoe
typedef struct SharedData
{
    int values[PLAYERS];
    pthread_barrier_t barrier;
    pthread_barrierattr_t attr;
    int player_counter;
} SharedData;

typedef struct ThreadData
{
    SharedData *data;
    int thread_id;
    pthread_t thread;
} ThreadData;

void rollTheDice(ThreadData *data)
{
    // rand() % (max_number + 1 - minimum_number) + minimum_number
    int rolled = (rand() % 6) + 1;
    data->data->values[data->thread_id] = rolled;
    printf("Player %d rolled a %d\n", data->thread_id, rolled);
}

void computeLoser(SharedData *data)
{
    int min = 6;
    for (int i = 0; i < PLAYERS; i++)
        if (data->values[i] != 0 && data->values[i] < min)
            min = data->values[i];

    for (int i = 0; i < PLAYERS; i++)
    {
        if (data->values[i] == min)
        {
            data->values[i] = PLAYER_LOST; // to know which player I should kill afterwards.
            data->player_counter--;
        }
    }
}

void *thread(void *arg)
{
    ThreadData *thread_data = arg;
    SharedData *shared = thread_data->data;

    do
    {
        // each thread should roll the dice
        rollTheDice(thread_data);

        // wait for each thread to finish rolling the dice and computing the looser(s)
        if (pthread_barrier_wait(&shared->barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
        {
            printf("---------------------\n");
            computeLoser(shared);

            // I don't know whether this is correct or not, found it on a website, I am trying to somehow change the count for the
            // barrier, because my barrier should wait for less threads now, because I am eliminating threads. If I don't do this, I will
            // wait forever. For some reason, it doesn't work as expected.
            pthread_barrierattr_setpshared(&thread_data->data->attr, thread_data->data->player_counter);

            // Another approach I tried was to destroy an re-create my barrier, didn't work either.
            // pthread_barrier_destroy(&shared->barrier);
            // if (pthread_barrier_init(&shared->barrier, NULL, shared->player_counter) != 0)
            // {
            //     perror("Could not create barrier");
            //     return NULL;
            // }
        }
        // I wait for each thread to reach this point, which means, everey player has rolled the dice and we already computed, who is the looser and
        // gets eliminated.
        pthread_barrier_wait(&shared->barrier);

        // Eliminating the player(s) who have lost.
        if (shared->values[thread_data->thread_id] == PLAYER_LOST)
        {
            printf("Eliminating player %i\n", thread_data->thread_id);
            pthread_exit(NULL);
        }
        pthread_barrier_wait(&thread_data->data->barrier);

    } while (thread_data->data->player_counter > 1); // game loop, continues until no players are left or we have a winner.

    for (size_t i = 0; i < PLAYERS; i++)
        if (thread_data->data->values[i] != PLAYER_LOST)
            printf("Player %ld has won the game!\n", i);

    return NULL;
}

int main()
{

    SharedData data = {0};
    data.player_counter = PLAYERS;

    ThreadData thread_data[PLAYERS];

    pthread_barrierattr_t attr;
    pthread_barrierattr_init(&attr);

    srand(time(NULL));
    if (pthread_barrier_init(&data.barrier, &attr, PLAYERS) != 0)
        return EXIT_FAILURE;

    data.attr = attr;
    for (int i = 0; i < PLAYERS; i++)
    {
        thread_data[i].data = &data;
        thread_data[i].thread_id = i;
        if (pthread_create(&thread_data[i].thread, NULL, thread, (void *)&thread_data[i]) != 0)
            return EXIT_FAILURE;
    }
    for (int i = 0; i < PLAYERS; i++)
        pthread_join(thread_data[i].thread, NULL);

    if (pthread_barrier_destroy(&data.barrier) != 0)
        return EXIT_FAILURE;
}
