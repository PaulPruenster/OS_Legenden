#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

// TODO: use header files
#include "allocator_tests.c"
#include "membench.c"

#define BLOCK_SIZE 1024

struct node
{
    char memory[BLOCK_SIZE];
    struct node *next;
};

typedef struct my_head_struct
{
    size_t size;        // how many blocks
    struct node *start; // first node
} head;

pthread_mutex_t mutex;        // for task 3 we don't need any mutex, because every thread has their own pool
_Thread_local head *storage;  // remove _Thread_local for task1

void *my_malloc(size_t size)
{
    // if required size is bigger then our BLOCK_SIZE
    // kind of useless, because we can assume, that we only allocate one block in each call
    if (size > BLOCK_SIZE)
    {
        return NULL;
    }
    pthread_mutex_lock(&mutex);

    // if there is no more free Blocks
    if (((head *)storage)->start == NULL)
    {
        return NULL;
    }
    // get the free node
    struct node *node = ((head *)storage)->start;


    //  set next free node of head
    ((head *)storage)->start = node->next;

    pthread_mutex_unlock(&mutex);

    // return pointer of the memory
    return node->memory;
}

void my_free(void *ptr)
{
    pthread_mutex_lock(&mutex);
    struct node *node;
    struct node *j;
    struct node *k;
    for (size_t i = 0; i < ((((head *)storage)->size - sizeof(head)) / sizeof(struct node)) - 1; i++)
    {
        node = (struct node *)(((ptrdiff_t)storage + sizeof(head) + (i * (sizeof(struct node)))));

        if (node->memory == ptr)
        {
            j = ((head *)storage)->start;
            if (j == NULL)
            {
                ((head *)storage)->start = node;

                node->next = NULL;
                pthread_mutex_unlock(&mutex);
                return;
            }
            if (j > node)
            {
                ((head *)storage)->start = node;
                node->next = j;
                pthread_mutex_unlock(&mutex);
                return;
            }
            k = j;
            j = j->next;

            while (j != NULL)
            {
                if (j > node)
                {
                    k->next = node;
                    node->next = j;
                    pthread_mutex_unlock(&mutex);
                    return;
                }
                k = j;
                j = j->next;
            }
            k->next = node;
            node->next = NULL;
            pthread_mutex_unlock(&mutex);
            return;
        }
    }
    pthread_mutex_unlock(&mutex);
}

void my_allocator_init(size_t size)
{
    if (size == 0)
    {
        perror("size should be greater than 0");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&mutex, NULL);

    storage = (head *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (storage == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    storage->size = size;
    storage->start = (struct node *)((ptrdiff_t)storage + sizeof(head));
    struct node *node = storage->start;
    while ((size_t)(ptrdiff_t)node < (ptrdiff_t)storage + size - sizeof(struct node))
    {
        node->next = (struct node *)((ptrdiff_t)node + sizeof(struct node));
        node = node->next;
    }
    node = NULL;
}

void my_allocator_destroy(void)
{
    munmap(storage, storage->size);
    pthread_mutex_destroy(&mutex);
}
int main(void)
{
   
    // test_free_list_allocator();
    run_membench_global(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
    //run_membench_thread_local(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
    return EXIT_SUCCESS;
}