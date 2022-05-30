#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>
#include "allocator_tests.h"
#include "membench.h"

#define BLOCK_SIZE 1024
typedef struct MemBlock
{
    char value[BLOCK_SIZE];
    struct MemBlock *next;
} MemBlock;

size_t gsize;
MemBlock *nextFree;
MemBlock *list;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *my_malloc(size_t size)
{
    pthread_mutex_lock(&mutex);
    if (size > BLOCK_SIZE)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if (nextFree == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    MemBlock *block = nextFree;
    nextFree = nextFree->next;
    pthread_mutex_unlock(&mutex);
    return (void *)block->value;
}

void my_free(void *ptr)
{
    pthread_mutex_lock(&mutex);
    if (ptr == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }
    MemBlock *block = (MemBlock *)ptr;
    block->next = nextFree;
    nextFree = block;
    pthread_mutex_unlock(&mutex);
}

void my_allocator_init(size_t size)
{
    gsize = size;
    list = (MemBlock *)mmap(NULL, size + sizeof(MemBlock), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (list == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    nextFree = list;

    size_t i = sizeof(MemBlock);

    while (i < size)
    {
        list->next = (MemBlock *)((char *)list + sizeof(MemBlock) - 1);
        list = list->next;
        i += sizeof(MemBlock);
    }

    list->next = NULL;
}

void my_allocator_destroy(void)
{
    munmap(list, gsize);
}

int main()
{
    test_free_list_allocator();
    run_membench_global(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
    return EXIT_SUCCESS;
}
