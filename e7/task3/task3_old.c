// link wos i gmoant hon: https://en.cppreference.com/w/c/thread/call_once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define PLAYERS 5


void *thread(void * param){
    // rand() % (max_number + 1 - minimum_number) + minimum_number
    int r = rand() % (6 + 1 - 1) + 1; 
    printf("Player %d rolled a %d\n", *(int *)param, r);
    return NULL;
}


int main() {

    pthread_t threads[PLAYERS];
    srand(time(NULL)); 
    for (int i = 0; i < PLAYERS; i++)
    {
        pthread_create(&threads[i], NULL, thread, (void*)&i);
    }
    for (int i = 0; i < PLAYERS; i++)
    {
        pthread_join(threads[i], NULL);
    }

}
