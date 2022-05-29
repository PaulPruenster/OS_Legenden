#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include "allocator_tests.c"
#include "membench.c"

#define BLOCK_SIZE 1024

struct node
{
    char memory[BLOCK_SIZE - 16];
    struct node *next;
    atomic_bool a;
};

typedef struct my_head_struct
{
    size_t size;        // how many blocks
    struct node *start; // first node
} head;

pthread_mutex_t mutex;
head *storage;

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
    //printf("storage : %p, start :%p\n", storage, storage->start);
    //printf("start = %d\n",(ptrdiff_t)(storage->start) - (ptrdiff_t) storage);
    // set next free node of head
    //node->isFree = false;
    ((head *)storage)->start = node->next;

    // return pointer of the memory
    pthread_mutex_unlock(&mutex);

    return node->memory;
}

void my_free(void *ptr)
{
    pthread_mutex_lock(&mutex);
    struct node *node;
    struct node *j;
    struct node *k;

    // struct node *node = (struct node *)(((ptrdiff_t)storage + sizeof(head)));
    for (size_t i = 0; i < ((((head *)storage)->size - sizeof(head)) / BLOCK_SIZE) - 2; i++)
    {
        node = (struct node *)(((ptrdiff_t)storage + sizeof(head) + (i * (sizeof(node)))));

        if (node->memory == ptr)
        {
            // memset(node->memory, (BLOCK_SIZE - 16), 0);
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
    
    //printf("Fehler  = storage : %p, start :%p, differenz: %d\n", storage, storage->start, (ptrdiff_t)node - (ptrdiff_t)storage);
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
    pthread_mutex_lock(&mutex);
    
    storage = (head *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (storage == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    storage->size = size;
    storage->start = (struct node *)((ptrdiff_t)storage + sizeof(head));
    struct node * test = storage->start;
    printf("storage : %p, last :%p\n", storage, (ptrdiff_t)storage + size);



    for (size_t i = 0; i < ((size - sizeof(head)) / BLOCK_SIZE) - 2; i++)
    {
        //printf("storage : %p, start :%p, differenz: %d\n", storage, storage->start, (((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node)))) - (ptrdiff_t)storage);
        ((struct node *)((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node))))->next = (struct node *)((ptrdiff_t)storage + sizeof(head) + ((i + 1) * (sizeof(struct node))));
        //((struct node *)((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node))))->isFree = true;
        //printf("storage : %p, start :%p, differenz: %d\n", storage, storage->start, (((ptrdiff_t)storage + sizeof(head) + (i * sizeof(struct node)))) - (ptrdiff_t)storage);

    }
    //printf("Bevor: storage : %p, start :%p, differenz: %d\n", storage, storage->start, ((((ptrdiff_t)storage + sizeof(head) + (((size / BLOCK_SIZE) - 1) * (sizeof(struct node)))))) - (ptrdiff_t)storage);
    ((struct node *)((ptrdiff_t)storage + sizeof(head) + (((size / BLOCK_SIZE) - 2) * (sizeof(struct node)))))->next = NULL;
    //((struct node *)((ptrdiff_t)storage + sizeof(head) + (((size / BLOCK_SIZE) - 2) * (sizeof(struct node)))))->isFree = true;   
    //printf("storage : %p, start :%p, differenz: %d\n", storage, storage->start, ((((ptrdiff_t)storage + sizeof(head) + (((size / BLOCK_SIZE) - 1) * (sizeof(struct node)))))) - (ptrdiff_t)storage);
    pthread_mutex_unlock(&mutex);
}

void my_allocator_destroy(void)
{
    munmap(storage, storage->size);
    pthread_mutex_destroy(&mutex);
}
// 268435456 - 268432400
// 268435456 - 268434448
// 268434448
int main(void)
{
    // 262141

    // task1: allocator_tests.c:15: test_free_list_allocator: Assertion `too_large == ((void *)0)' failed.
    //my_allocator_init(1024 * 10);
    //void* ptr1 = my_malloc(1);
    //int j = 1;
   
    
    //test_free_list_allocator();
    run_membench_thread_local(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
    return EXIT_SUCCESS;
}
//1048560