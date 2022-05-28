#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include "allocator_tests.c"

#define BLOCK_SIZE 1024

pthread_mutex_t mutex;
void *storage;
struct node
{
    char memory[BLOCK_SIZE - 16];
    struct node *next_asdf;
    atomic_bool isFree;
};

typedef struct my_head_struct
{
    size_t size; // how many blocks
    struct node *start; // first node
} head;

void *my_malloc(size_t size)
{
    // if required size is bigger then our BLOCK_SIZE
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

    // set next free node of head
    node->isFree = false;
    ((head *)storage)->start = node->next_asdf;

    // return pointer of the memory
    pthread_mutex_unlock(&mutex);

    return node->memory;
}

void my_free(void *ptr)
{
    pthread_mutex_lock(&mutex);

    struct node *j;

    for (size_t i = 0; ((head *)storage)->size; i++)
    {
        struct node *node = (struct node*) (((ptrdiff_t)storage + sizeof(head) + (i * (sizeof(node)))));

        if (node->memory == ptr && !node->isFree)
        {
            //memset(node->memory, BLOCK_SIZE, 0);
            j = ((head *)storage)->start;
            if (j == NULL)
            {
                ((head *)storage)->start = node;
                node->next_asdf = NULL;
                pthread_mutex_unlock(&mutex);
                return;
            }
            if (j > node)
            {
                ((head *)storage)->start = node;
                node->next_asdf = j;
                pthread_mutex_unlock(&mutex);
                return;
            }
            while (j != NULL)
            {
                if (j < node)
                {
                    node->next_asdf = j->next_asdf;
                    j->next_asdf = node;
                    pthread_mutex_unlock(&mutex);
                    return;
                }
                if (j > node)
                {
                    j-> next_asdf = node;
                    node->next_asdf = NULL;
                    pthread_mutex_unlock(&mutex);
                    return;
                }
                j = j->next_asdf;
            }
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
    storage  = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (storage == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    ((head *)storage)->size = size;
    ((head *)storage)->start =(struct node*) ((ptrdiff_t)storage + sizeof(head));

    for (size_t i = 0; i < ((size - sizeof(head)) / BLOCK_SIZE); i++)
    {
        ((struct node *)((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node))))->next_asdf = (struct node*)((ptrdiff_t)storage + sizeof(head) + ((i + 1) * (sizeof(struct node))));
        ((struct node *)((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node))))->isFree = true;
    }
    ((struct node *)((ptrdiff_t)storage + sizeof(head) + ((size / BLOCK_SIZE)-1 * (sizeof(struct node)))))->next_asdf = NULL;
}

void my_allocator_destroy(void)
{
    munmap(storage, ((head *)storage)->size * (sizeof(struct node)) + sizeof(head));
    pthread_mutex_destroy(&mutex);
}

int main(void) {
    test_free_list_allocator(); 
    return EXIT_SUCCESS;
}