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
    char *memory;
    size_t size;
    struct node *next; // pointer to next element
    struct node *prev; // pointer to the previous element
    atomic_bool isFree;
};

typedef struct my_head_struct
{
    size_t size;        // how many blocks
    struct node *start; // first node
    size_t free_space;  // indicates how much storage/bytes are left.

} head;

pthread_mutex_t mutex;
head *storage;

void *my_malloc(size_t size)
{
    // if required size is bigger then our free space left
    if (size > storage->free_space)
    {
        return NULL;
    }
    char a[size - sizeof(struct node)]; // create a array/pointer, which consumes the given memory
    pthread_mutex_lock(&mutex);

    // get the free node
    struct node *freeblock = storage->start;
    while (freeblock != NULL)
    {
        if (freeblock->isFree && freeblock->size >= size)
            break;
        freeblock = freeblock->next;
    }
    if (freeblock == NULL) // no free space left
        return NULL;

    // split the free pace node into two and return the reseved one
    struct node *allocated = freeblock;

    freeblock->memory += size; //????
    storage->free_space -= size;

    pthread_mutex_unlock(&mutex);

    // return pointer of the memory
    return allocated->memory;
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
    storage->free_space = size - sizeof(head);
    storage->start = (struct node *)((ptrdiff_t)storage + sizeof(head));
    storage->start->size = size - sizeof(head);
    storage->start->isFree = true;
    storage->start->next = NULL;
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
    return EXIT_SUCCESS;
}