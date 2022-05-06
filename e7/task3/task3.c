#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define PLAYER_LOST 7
#define PLAYER_DEAD 8
#define PLAYERS 5

// https://godbolt.org/z/3YdsEvPoe
typedef struct SharedData
{
    int values[PLAYERS];
    pthread_barrier_t barrier;
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
        if (shared->values[thread_data->thread_id] != PLAYER_DEAD)
            rollTheDice(thread_data);

        // wait for each thread to finish rolling the dice and computing the looser(s)
        if (pthread_barrier_wait(&shared->barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
            computeLoser(shared);

        // all threads wait for computeUser and then eliminate themself
        pthread_barrier_wait(&shared->barrier); // ?: do we need to wait for it?

        // Eliminating the player(s) who have lost.
        if (shared->values[thread_data->thread_id] == PLAYER_LOST)
        {
            printf("Eliminating player %i\n", thread_data->thread_id);
            shared->values[thread_data->thread_id] = PLAYER_DEAD;
        }

        if (pthread_barrier_wait(&shared->barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
            printf("---------------------\n");

    } while (thread_data->data->player_counter > 1); // game loop, continues until no players are left or we have a winner.

    if (shared->player_counter == 0)
        printf("All players were eliminated!\n");

    if (thread_data->data->values[thread_data->thread_id] != PLAYER_DEAD)
        printf("Player %d has won the game!\n", thread_data->thread_id);

    return NULL;
}

int main()
{

    SharedData data = {0};
    data.player_counter = PLAYERS;

    ThreadData thread_data[PLAYERS];

    srand(time(NULL));
    if (pthread_barrier_init(&data.barrier, NULL, PLAYERS) != 0)
        return EXIT_FAILURE;

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
