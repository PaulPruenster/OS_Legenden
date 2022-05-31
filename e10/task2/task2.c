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

/*
DISCLAIMER: this solution does not work, but we spend a couple of hours trying to fix it.

 Our idea was to create a double linked list, which points to every node/element in the memory.

 malloc: find the (first) free node of the list, where the given size fits in. Afterwards we split the node
         into to separates notes. One remains still free and the other contains the new malloced data.

 free:   we are trying to find the node, which stores the given address. Afterwards we check the direct neighbours
         in order to merge free blocks. We don't have to check any other notes, because we can assume that they
         are already merged if possible.

*/



struct node {
    char *memory;
    size_t size;
    struct node *next; // pointer to next element
    struct node *prev; // pointer to the previous element
    atomic_bool isFree;
};

typedef struct my_head_struct {
    size_t size;        // how many blocks
    struct node *start; // first node
    size_t free_space;  // indicates how much storage/bytes are left.

} head;

pthread_mutex_t mutex;
head *storage;

void *my_malloc(size_t size) {
    // if required size is bigger then our free space left
    if (size > storage->free_space) {
        return NULL;
    }
    pthread_mutex_lock(&mutex);

    // get the free node
    struct node *freeblock = storage->start;
    while (freeblock != NULL) {
        if (freeblock->isFree && freeblock->size >= size)
            break;
        freeblock = freeblock->next;
    }
    if (freeblock == NULL) // no free space left
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    // split the free pace node into two and return the reseved one
    struct node allocated = {0};
    allocated.size = size;
    allocated.next = freeblock;
    allocated.prev = freeblock->prev;
    allocated.isFree = false;
    // verschieben um die allozierte memory
    freeblock->memory += sizeof(struct node) + size; //????
    storage->free_space -= (size + (ptrdiff_t) freeblock + sizeof(struct node));

    pthread_mutex_unlock(&mutex);

    // return pointer of the memory
    return allocated.memory;
}

void merge_Block(struct node *first, struct node *next) {
    first->size += next->size;
    first->next = next->next;
    next = NULL;
}

void my_free(void *ptr) {
    pthread_mutex_lock(&mutex);

    struct node *freeblock = storage->start;
    while (freeblock != NULL) {
        if (freeblock->memory == ptr) {
            if (!freeblock->isFree) {
                perror("can't free a empty block");
                return;
            }
            break;
        }
        freeblock = freeblock->next;
    }
    freeblock->isFree = true;

    // merge neighbours if they are both free
    if (freeblock->prev->isFree) {
        merge_Block(freeblock->prev, freeblock);
    }
    if (freeblock->next->isFree) {
        merge_Block(freeblock, freeblock->next);
    }

    pthread_mutex_unlock(&mutex);
    return;
}

void my_allocator_init(size_t size) {
    if (size == 0) {
        perror("size should be greater than 0");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&mutex, NULL);

    storage = (head *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (storage == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    storage->size = size;
    storage->free_space = size - sizeof(head);
    storage->start = (struct node *) ((ptrdiff_t) storage + sizeof(head));
    storage->start->size = size - sizeof(head);
    storage->start->isFree = true;
    storage->start->next = NULL;
}

void my_allocator_destroy(void) {
    munmap(storage, storage->size);
    pthread_mutex_destroy(&mutex);
}

int main(void) {

    // test_free_list_allocator();
    test_best_fit_allocator();
    //run_membench_global(my_allocator_init, my_allocator_destroy, my_malloc, my_free);
    return EXIT_SUCCESS;
}